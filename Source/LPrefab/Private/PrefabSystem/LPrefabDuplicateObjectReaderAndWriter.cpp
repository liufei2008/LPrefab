// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefabObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "LPrefabModule.h"

namespace LPrefabSystem
{
	FLPrefabDuplicateObjectWriter::FLPrefabDuplicateObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLPrefabObjectWriter(Bytes, InSerializer, InSkipPropertyNames)
	{
		
	}
	bool FLPrefabDuplicateObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
		{
			return true;
		}

		return false;
	}
	bool FLPrefabDuplicateObjectWriter::SerializeObject(UObject* Object)
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
			bool canSerializeObject = false;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializeObject = true;
				//MapObjectToGuid could be passed-in, if that the CollectObjectToSerailize will not execute which will miss some objects. so we still need to collect objects to serialize
				FGuid guid;
				Serializer.CollectObjectToSerailize(Object, guid);
			}
			else
			{
				FGuid guid;
				canSerializeObject = Serializer.CollectObjectToSerailize(Object, guid);
				if (canSerializeObject)
				{
					guidPtr = &guid;
				}
			}

			if (canSerializeObject)//object belongs to this actor hierarchy
			{
				auto type = (uint8)EObjectType::ObjectReference;
				*this << type;
				*this << *guidPtr;
				return true;
			}
			else//object not belongs to this actor hierarchy, just copy pointer
			{
				auto type = (uint8)EObjectType::NativeSerailizeForDuplicate;
				*this << type;
				ByteOrderSerialize(&Object, sizeof(Object));
				return true;
			}
		}
	}
	FString FLPrefabDuplicateObjectWriter::GetArchiveName() const
	{
		return TEXT("FLPrefabDuplicateObjectReader");
	}



	FLPrefabDuplicateObjectReader::FLPrefabDuplicateObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLPrefabObjectReader(Bytes, InSerializer, InSkipPropertyNames)
	{

	}
	bool FLPrefabDuplicateObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
		{
			return true;
		}

		return false;
	}
	bool FLPrefabDuplicateObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
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
		case LPrefabSystem::EObjectType::NativeSerailizeForDuplicate:
		{
			ByteOrderSerialize(&Object, sizeof(Object));
			return true;
		}
		break;
		}
		return false;
	}
	FString FLPrefabDuplicateObjectReader::GetArchiveName() const
	{
		return TEXT("FLPrefabDuplicateObjectReader");
	}
}
