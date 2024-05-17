// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ArchiveSerializedPropertyChain.h"

namespace LPrefabSystem
{
	class ActorSerializerBase;

	enum class EObjectType :uint8
	{
		None,
		/** Asset resource */
		Asset,
		/** UClass */
		Class,
		/** UObject reference(Not asset), include actor/ component/ uobject */
		ObjectReference,
		/** UFunction reference, currently for PrefabAnimation's event track. */
		Function,
		/** K2Node reference, currently for PrefabAnimation's event track. */
		K2Node,
		/** Only for duplicate, use native ObjectWriter/ObjectReader serialization method */
		NativeSerailizeForDuplicate,
	};

	/** 
	 * If not have valid property chain, then it is member property.
	 * Why use a template instead of FArchiveState? Because FArchiveState's construcion is prirvate, I can't convert FLPrefabObjectWriterXXX to FArchiveState.
	 */
	template<class T>
	bool CurrentIsMemberProperty(const T& t)
	{
		auto PropertyChain = t.GetSerializedPropertyChain();
		if (PropertyChain == nullptr || PropertyChain->GetNumProperties() == 0)
		{
			return true;
		}
		return false;
	}
	bool LPrefab_ShouldSkipProperty(const FProperty* InProperty);

	class LPREFAB_API FLPrefabObjectWriter : public FObjectWriter
	{
	public:
		FLPrefabObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FObjectPtr& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		ActorSerializerBase& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class LPREFAB_API FLPrefabObjectReader : public FObjectReader
	{
	public:
		FLPrefabObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FObjectPtr& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		ActorSerializerBase& Serializer;
		TSet<FName> SkipPropertyNames;
	};

	class LPREFAB_API FLPrefabDuplicateObjectWriter : public FLPrefabObjectWriter
	{
	public:
		FLPrefabDuplicateObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LPREFAB_API FLPrefabDuplicateObjectReader : public FLPrefabObjectReader
	{
	public:
		FLPrefabDuplicateObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};



	class LPREFAB_API FLPrefabOverrideParameterObjectWriter : public FLPrefabObjectWriter
	{
	public:
		FLPrefabOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		mutable TSet<FName> OverridePropertyNames;
	};
	class LPREFAB_API FLPrefabOverrideParameterObjectReader : public FLPrefabObjectReader
	{
	public:
		FLPrefabOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		mutable TSet<FName> OverridePropertyNames;
	};


	class LPREFAB_API FLPrefabDuplicateOverrideParameterObjectWriter : public FLPrefabOverrideParameterObjectWriter
	{
	public:
		FLPrefabDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LPREFAB_API FLPrefabDuplicateOverrideParameterObjectReader : public FLPrefabOverrideParameterObjectReader
	{
	public:
		FLPrefabDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};


	class LPREFAB_API FLPrefabImmediateOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLPrefabImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
	class LPREFAB_API FLPrefabImmediateOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLPrefabImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
}