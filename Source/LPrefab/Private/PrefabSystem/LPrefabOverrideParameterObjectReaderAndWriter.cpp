// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefabObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Engine/Blueprint.h"
#include "LPrefabModule.h"

namespace LPrefabSystem
{
	FLPrefabOverrideParameterObjectWriter::FLPrefabOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLPrefabObjectWriter(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLPrefabOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	bool FLPrefabOverrideParameterObjectWriter::SerializeObject(UObject* Object)
	{
		if (auto Function = Cast<UFunction>(Object))
		{
			auto OuterClass = Function->GetTypedOuter<UClass>();
			if (OuterClass != nullptr)
			{
				auto FunctionName = Function->GetFName();
				auto FunctionNameId = Serializer.FindOrAddNameFromList(FunctionName);
				auto OuterClassId = Serializer.FindOrAddClassFromList(OuterClass);
				auto type = (uint8)EObjectType::Function;
				*this << type;
				*this << OuterClassId;
				*this << FunctionNameId;
				return true;
			}
			return false;
		}
		//if (auto K2Node = Cast<UK2Node>(Object))//K2Node is editor only, so we just check the name
		if (Object->GetName().StartsWith(TEXT("K2Node_")))
		{
			auto OuterObject = Object->GetTypedOuter<UBlueprint>();
			if (OuterObject != nullptr)
			{
				auto NodeName = Object->GetFName();
				auto NameId = Serializer.FindOrAddNameFromList(NodeName);
				auto OuterObjectId = Serializer.FindOrAddAssetIdFromList(OuterObject);
				auto type = (uint8)EObjectType::K2Node;
				*this << type;
				*this << OuterObjectId;
				*this << NodeName;
				return true;
			}
			return false;
		}
		if (Object->IsAsset() && !Object->GetClass()->IsChildOf(AActor::StaticClass()))
		{
			auto id = Serializer.FindOrAddAssetIdFromList(Object);
			auto type = (uint8)EObjectType::Asset;
			*this << type;
			*this << id;
			return true;
		}
		else
		{
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				auto type = (uint8)EObjectType::ObjectReference;
				*this << type;
				*this << *guidPtr;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	FString FLPrefabOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLPrefabOverrideParameterObjectWriter");
	}


	FLPrefabOverrideParameterObjectReader::FLPrefabOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLPrefabObjectReader(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLPrefabOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	bool FLPrefabOverrideParameterObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
	{
		uint8 typeUint8 = 0;
		*this << typeUint8;
		auto type = (EObjectType)typeUint8;
		switch (type)
		{
		case LPrefabSystem::EObjectType::Class:
		{
			check(CanSerializeClass);
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindClassFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LPrefabSystem::EObjectType::Asset:
		{
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindAssetFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LPrefabSystem::EObjectType::Function:
		{
			int32 OuterClasstId = -1;
			int32 FunctionNameId = -1;
			*this << OuterClasstId;
			*this << FunctionNameId;
			auto OuterClass = Serializer.FindClassFromListByIndex(OuterClasstId);
			auto FunctionName = Serializer.FindNameFromListByIndex(FunctionNameId);
			Object = OuterClass->FindFunctionByName(FunctionName);
			return true;
		}
		break;
		case LPrefabSystem::EObjectType::K2Node:
		{
			int32 OuterObjectId = -1;
			int32 NodeNameId = -1;
			*this << OuterObjectId;
			*this << NodeNameId;
			if (OuterObjectId != -1 && NodeNameId != -1)
			{
				auto OuterObject = Serializer.FindAssetFromListByIndex(OuterObjectId);
				if (OuterObject != nullptr)
				{
					auto NodeName = Serializer.FindNameFromListByIndex(NodeNameId);
					ForEachObjectWithOuter(OuterObject, [&NodeName, &Object](UObject* ItemObject) {
						if (NodeName == ItemObject->GetFName())
						{
							Object = ItemObject;
						}
						});
					return true;
				}
			}
		}
		break;
		case LPrefabSystem::EObjectType::ObjectReference:
		{
			FGuid guid;
			*this << guid;
			if (auto ObjectPtr = Serializer.MapGuidToObject.Find(guid))
			{
				Object = *ObjectPtr;
				return true;
			}
		}
		break;
		}
		return false;
	}
	FString FLPrefabOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLPrefabOverrideParameterObjectReader");
	}





	FLPrefabImmediateOverrideParameterObjectWriter::FLPrefabImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLPrefabImmediateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	FString FLPrefabImmediateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLPrefabImmediateOverrideParameterObjectWriter");
	}


	FLPrefabImmediateOverrideParameterObjectReader::FLPrefabImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLPrefabImmediateOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	FString FLPrefabImmediateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLPrefabImmediateOverrideParameterObjectReader");
	}
}
