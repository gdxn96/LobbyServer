#pragma once
#include <type_traits>
#include <string>
#include "dependancies\rapidjson\rapidjson.h"
#include "dependancies\rapidjson\document.h"
#include "dependancies\rapidjson\prettywriter.h"
#include <unordered_map>
#include "JsonMetaHandler.h"
#include "RuntimeEditable.h"
#include "FileMonitor.h"



struct Variable;
class MetaType;

#define META_DEBUGGING false


#define DEBUG(x) if (META_DEBUGGING) { std::cerr << x << std::endl; }

// Takes a pointer and returns a pointer offset in bytes
#define PTR_ADD(PTR, OFFSET) \
  ((void *)(((char *)(PTR)) + (OFFSET)))

typedef std::string(*SerializeFn)(void*, const MetaType* m);
typedef void(*DeSerializeFn)(void*, rapidjson::Value&, const MetaType*);


#define DEFINE_META(T) \
  static MetaCreator<std::remove_cv<T>::type> UNIQUE_NAME_GENERATOR()(#T, sizeof(T)); \
  std::remove_cv<T>::type *T::NullCast() { return reinterpret_cast<std::remove_cv<T>::type *>(NULL); } \
  void T::AddMember(std::string name, unsigned offset, MetaType *data, bool re) { return MetaCreator<std::remove_cv<T>::type>::AddMember(name, offset, data, re); } \
  void MetaCreator<std::remove_cv<T>::type>::RegisterMetaType() { T::RegisterMetaType(); } \
  void T::RegisterMetaType()

// META_DATA
// Purpose : This macro goes on the inside of a class within the public section. It declares
//           a few member functions for use by the MetaType system to retrieve information about
//           the class.
#define META_DATA(T) \
  static void AddMember(std::string name, unsigned offset, MetaType *data, bool re=false); \
  static std::remove_cv<T>::type *NullCast(); \
  static void RegisterMetaType()

// Defines the RegisterMetaType for you
#define DEFINE_META_PRIMITIVE(T) \
  static MetaCreator<std::remove_cv<T>::type> UNIQUE_NAME_GENERATOR()(#T, sizeof(T)); \
  inline void MetaCreator<std::remove_cv<T>::type>::RegisterMetaType() \
  { \
    MetaCreator<std::remove_cv<T>::type>::SetSerializeFn(MetaType::ToJson<std::remove_cv<T>::type>); \
    MetaCreator<std::remove_cv<T>::type>::SetDeSerializeFn(MetaType::FromJson<std::remove_cv<T>::type>); \
  }

#define ADD_MEMBER(MEMBER) \
  AddMember(#MEMBER, (unsigned)(&(NullCast()->MEMBER)), META(NullCast()->MEMBER))

//add runtime editable member
#define ADD_RE_MEMBER(MEMBER) \
  AddMember(#MEMBER, (unsigned)(&(NullCast()->MEMBER)), META(NullCast()->MEMBER), true)

//this macro can only work with default constructable objects
#define LINK_TO_JSON(OBJECT) \
  std::string x = #OBJECT; \
  std::string jsonPath = std::string("json/") + x + std::string(".json"); \
  JsonHandler::get<OBJECT>(jsonPath, true); \
  FileMonitor::getInstance().WatchFile(jsonPath, std::bind(&RuntimeEditable<std::remove_cv<OBJECT>::type>::reloadJson));

#define PASTE_TOKENS( _, __ ) _##__
#define NAME_GENERATOR_INTERNAL( _ ) PASTE_TOKENS( GENERATED_TOKEN_, _ )
#define UNIQUE_NAME_GENERATOR( ) NAME_GENERATOR_INTERNAL( __COUNTER__ )

//
// META_TYPE
// Purpose: Retrieves the proper MetaType instance of an object by type.
//
#define META_TYPE(T) (MetaCreator<std::remove_cv<T>::type>::Get())

//
// META
// Purpose: Retrieves the proper MetaType instance of an object by an object's type.
//
#define META(OBJECT) (MetaCreator<std::remove_cv<decltype(OBJECT)>::type>::Get())

class MetaType;

//
// Member
// Purpose: Stores information (name and offset of member) about a data member of a specific class. Multiple
//          Member objects can be stored in MetaType objects within a std::vector.
//
class Member
{
public:
	Member(std::string string, unsigned val, MetaType *meta, bool re=false);
	~Member();

	const std::string &Name() const; // Gettor for name
	unsigned Offset() const; // Gettor for offset
	const MetaType *Meta() const; // Gettor for data

	Member *& Next();
	Member *const& Next() const;

	bool IsRuntimeEditable() const;

private:
	std::string name;
	unsigned offset;
	const MetaType *data;
	Member *next;
	bool runtimeEditable;
};

// MetaType
// Purpose: Object for holding various info about any C++ type for the MetaType reflection system.
//
class MetaType
{
public:
	MetaType(std::string string = "", unsigned val = 0);
	~MetaType();

	void Init(std::string string, unsigned val);

	const std::string& Name() const;
	
	unsigned Size() const;
	void AddMember(const Member *member);
	bool HasMembers() const;

	void Copy(void *data, const void *src) const;
	void Delete(void *data) const;
	void *NewCopy(const void *src) const;
	void *New() const;

	const Member *Members() const;

	void SetSerialize(SerializeFn fn = NULL);
	void SetDeSerialize(DeSerializeFn fn = NULL);
	std::string MetaType::Serialize(void* v, const MetaType * m=nullptr, bool reOnly=false) const;
	void MetaType::DeSerialize(void* v, rapidjson::Value& val, const MetaType * m = nullptr, bool reOnly=false) const;

#pragma region ToJson
	const std::string ToJson(Variable& var, bool reOnly=false) const;

	template<typename T>
	static std::string ToJson(void* v, const MetaType* m) 
	{ 
		if (m != nullptr)
		{
			DEBUG("Serialization not supported for type: " + m->m_name);
		}
		else
		{
			DEBUG("Serialization not supported for unregistered type");
		}
		return "null"; 
	};

	static std::string ToJson(void* v, const MetaType* m, bool reOnly=false);

	template<>
	static std::string ToJson<int>(void* val, const MetaType* m) {
		static char buf[65];
		sprintf_s(buf, "%d", *(int*)val);
		return buf;
	}

	template<>
	static std::string ToJson<bool>(void* val, const MetaType* m) {
		bool boolean = *(bool*)val;
		if (boolean)
		{
			return "true";
		}
		else
		{
			return "false";
		}
	}

	template<>
	static std::string ToJson<double>(void* val, const MetaType* m) {
		static char buf[65];
		sprintf_s(buf, "%d", *(double*)val);
		return buf;
	}

	template<>
	static std::string ToJson<long>(void* val, const MetaType* m) {
		static char buf[65];
		sprintf_s(buf, "%d", *(long*)val);
		return buf;
	}

	template<>
	static std::string ToJson<float>(void* val, const MetaType* m) {
		static char buf[65];
		sprintf_s(buf, "%d", *(float*)val);
		return buf;
	}

	template<>
	static std::string ToJson<char *>(void* val, const MetaType* m) {
		std::string result = "\"";
		result += (*(char**)(val));
		return result + "\"";
	}

	template<>
	static std::string ToJson<const char*>(void* val, const MetaType* m) {
		std::string result = "\"";
		result += (*(char**)(val));
		return result + "\"";
	}

	template<>
	static std::string ToJson<std::string>(void* val, const MetaType* m) {
		std::string result = "\"";
		result += (*(std::string*)(val));
		return result + "\"";
	}
#pragma endregion

	
#pragma region FromJson
	static void FromJson(rapidjson::Value& src, void* dest, const MetaType* type, bool reOnly)
	{
		if (type->HasMembers())
		{
			const Member* mem = type->Members();
			while (mem)
			{
				if (!reOnly || mem->IsRuntimeEditable())
				{
					assert(src.IsObject());
					assert(src.HasMember(mem->Name().c_str()));//json member not present in object

					rapidjson::Value& memberVal = src[mem->Name().c_str()];
					void* memberDest = PTR_ADD(dest, mem->Offset());
					mem->Meta()->DeSerialize(memberDest, memberVal, mem->Meta());
				}

				mem = mem->Next();
			}
		}
		else
		{
			type->DeSerialize(dest, src, type, reOnly);
		}
	}
	template<typename T>
	static void FromJson(void* v, rapidjson::Value&, const MetaType* m) 
	{
		DEBUG("DeSerialization not supported for type: " + m->m_name);
	};

	template<>
	static void FromJson<int>(void* v, rapidjson::Value& rVal, const MetaType* m)
	{
		*(int*)v = rVal.GetInt();
	}

	template<>
	static void FromJson<long>(void* v, rapidjson::Value& rVal, const MetaType* m)
	{
		*(long*)v = rVal.GetInt();
	}

	template<>
	static void FromJson<bool>(void* v, rapidjson::Value& rVal, const MetaType* m)
	{
		*(bool*)v = rVal.GetBool();
	}

	//char* and const char* are weird, maybe I can't modify the memory directly?
	//template<>
	//static void FromJson<char*>(void* v, rapidjson::Value& rVal, const MetaType* m)
	//{
	//	const char * string = static_cast<char*> (v);
	//	string = rVal.GetString();
	//}

	//template<>
	//static void FromJson<const char*>(void* v, rapidjson::Value& rVal, const MetaType* m)
	//{
	//	const char * string = static_cast<char*> (v);
	//	string = rVal.GetString();
	//}

	template<>
	static void FromJson<std::string>(void* v, rapidjson::Value& rVal, const MetaType* m)
	{
		if (rVal.GetStringLength() > 0)
			*(std::string*)v = rVal.GetString();
	}

	template<>
	static void FromJson<float>(void* v, rapidjson::Value& rVal, const MetaType* m)
	{
		*(float*)v = rVal.GetFloat();
	}
#pragma endregion

	

private:
	SerializeFn serialize;
	DeSerializeFn deserialize;
	Member *m_members;
	Member *m_lastMember;
	std::string m_name;
	unsigned m_size;
};

inline std::string PrettifyJson(rapidjson::Document & val)
{
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	val.Accept(writer);
	std::string json = buffer.GetString();
	return json;
}

struct Variable
{
	Variable() :
		address(NULL),
		type(NULL)
	{}

	Variable(void *d, const MetaType *m) : address(d), type(m)
	{

	}

	template<typename Var>
	Variable(Var* var) :
		address(var),
		type(META_TYPE(Var))
	{}

	template <typename T>
	void FromJson(std::string json, bool re=false)
	{
		if (json != "")
		{
			rapidjson::Document document;
			document.Parse(json.c_str());
			assert(document.IsObject());
			type->FromJson(document, address, type, re);
		}
	}

	std::string ToJson(bool runtimeEditableOnly=false)
	{
		std::string json = type->ToJson(*this, runtimeEditableOnly);
		rapidjson::Document d;
		d.Parse(json.c_str());
		json = PrettifyJson(d);
		return json;
	}

	template <typename T>
	T& get()
	{
		return *reinterpret_cast<T *>(address);
	}

	void* getAddress()
	{
		return address;
	}

	Variable& operator=(const Variable& rhs)
	{
		if (this == &rhs)
			return *this;

		if (type)
		{
			if (type == rhs.type)
				type->Copy(address, rhs.address);
			else
			{
				assert(rhs.type); // Cannot make an instance of NULL meta!

				type->Delete(address);
				type = rhs.type;

				// We require a new copy if meta does not match!
				if (type)
					address = type->NewCopy(&rhs.address);
			}
		}

		return *this;
	}

	void*  address;
	const MetaType* type;
};

template <typename Type>
class MetaCreator
{
public:
	MetaCreator(std::string name, unsigned size)
	{
		Init(name, size);
	}

	static void Init(std::string name, unsigned size)
	{
		Get()->Init(name, size);
		RegisterMetaType();
	}

	static void AddMember(std::string memberName, unsigned memberOffset, MetaType *meta, bool re=false)
	{
		Get()->AddMember(new Member(memberName, memberOffset, meta, re));
	}

	static void SetSerializeFn(SerializeFn fn)
	{
		Get()->SetSerialize(fn);
	}

	static void SetDeSerializeFn(DeSerializeFn fn)
	{
		Get()->SetDeSerialize(fn);
	}

	static Type *NullCast()
	{
		return reinterpret_cast<Type *>(NULL);
	}

	static void RegisterMetaType();

	// Ensure a single instance can exist for this class type
	static MetaType *Get()
	{
		static MetaType instance;
		return &instance;
	}
};

inline const std::string MetaType::ToJson(Variable& var, bool reOnly) const
{
	std::string result;
	if (HasMembers())
	{
		result += "{";
		const Member *mem = m_members;
		while (mem)
		{
			if (!reOnly || mem->IsRuntimeEditable())
			{
				void *offsetData = PTR_ADD(var.getAddress(), mem->Offset());
				Variable member = Variable(offsetData, mem->Meta());
				result += "\"" + mem->Name() + "\"" + ":" + member.type->Serialize(member.getAddress(), member.type, reOnly);
			}
			if (mem->Next())
			{
				if ((mem->IsRuntimeEditable() && mem->Next()->IsRuntimeEditable()) || !reOnly)
				{
					result += ",";
				}
			}

			mem = mem->Next();
		}

		result += "}";
	}
	else
	{
		result = var.type->Serialize(var.getAddress(), nullptr, reOnly);
	}

	return result;
}


//---------------------------------------------------------

inline Member::Member(std::string string, unsigned val, MetaType *meta, bool re) : name(string), offset(val), data(meta), next(NULL), runtimeEditable(re)
{
}

inline Member::~Member()
{
}

inline const std::string& Member::Name() const
{
	return name;
}

inline unsigned Member::Offset() const
{
	return offset;
}

inline const MetaType *Member::Meta() const
{
	return data;
}

inline Member *& Member::Next()
{
	return next;
}

inline Member *const& Member::Next() const
{
	return next;
}

inline bool Member::IsRuntimeEditable() const
{
	return runtimeEditable;
}

inline MetaType::MetaType(std::string string, unsigned val) : m_name(string), m_size(val), m_members(NULL), m_lastMember(NULL)
{
}

inline MetaType::~MetaType()
{
}

inline void MetaType::Init(std::string string, unsigned val)
{
	m_name = string;
	m_size = val;
}

inline const std::string& MetaType::Name() const
{
	return m_name;
}

inline unsigned MetaType::Size() const
{
	return m_size;
}

inline void MetaType::AddMember(const Member *member)
{
	if (!m_members)
		m_members = const_cast<Member *>(member);
	else
		m_lastMember->Next() = const_cast<Member *>(member);

	m_lastMember = const_cast<Member *>(member);
}

inline bool MetaType::HasMembers() const
{
	return (m_members) ? true : false;
}

inline void MetaType::Copy(void *dest, const void *src) const
{
	memcpy(dest, src, m_size);
}

inline void MetaType::Delete(void *data) const
{
	delete[] reinterpret_cast<char *>(data);
	data = NULL;
}

inline void *MetaType::NewCopy(const void *src) const
{
	void *data = new char[m_size];
	memcpy(data, src, m_size);
	return data;
}

inline void *MetaType::New() const
{
	return new char[m_size];
}

inline const Member *MetaType::Members() const
{
	return m_members;
}

inline void MetaType::SetSerialize(SerializeFn fn)
{
	serialize = fn;
}

inline void MetaType::SetDeSerialize(DeSerializeFn fn)
{
	deserialize = fn;
}

inline std::string MetaType::Serialize(void* v, const MetaType * m, bool reOnly) const
{
	if (serialize)
		return serialize(v, m);
	else
	{
		if (m != nullptr)
		{
			return ToJson(v, m, reOnly);
		}
		else
		{
			return ToJson<void>(v, m);
		}
	}	
}

inline void MetaType::DeSerialize(void * v, rapidjson::Value & val, const MetaType * m, bool reOnly) const
{
	if (deserialize)
		deserialize(v, val, m);
	else
		if (m != nullptr && m->Size() != 0)
			FromJson(val, v, m, reOnly);
		else
			FromJson<void>(v, val, m);
}

inline std::string MetaType::ToJson(void* v, const MetaType* m, bool reOnly) { return m->ToJson(Variable(v, m), reOnly); }

DEFINE_META_PRIMITIVE(int);
DEFINE_META_PRIMITIVE(std::string);
DEFINE_META_PRIMITIVE(float);
DEFINE_META_PRIMITIVE(const char*);
DEFINE_META_PRIMITIVE(char*);
DEFINE_META_PRIMITIVE(char);
DEFINE_META_PRIMITIVE(signed char);
DEFINE_META_PRIMITIVE(short int);
DEFINE_META_PRIMITIVE(long int);
DEFINE_META_PRIMITIVE(unsigned char);
DEFINE_META_PRIMITIVE(unsigned short int);
DEFINE_META_PRIMITIVE(unsigned int);
DEFINE_META_PRIMITIVE(unsigned long int);
DEFINE_META_PRIMITIVE(wchar_t);
DEFINE_META_PRIMITIVE(bool);
DEFINE_META_PRIMITIVE(double);
DEFINE_META_PRIMITIVE(long double);