#pragma once

#include <string>
#include <map>
#include "smart_ptr.h"

template <class TDataType> class CCacheObject
{

};

template <class TIndexType> class IDataCacheIndexMap
{
public:
//	GetBy()
};

template <class TDataType, class TIndexType> class CCacheIndex
{
public:
	CCacheIndex();

	typedef CCacheObject<TDataType> TCacheObject;
	typedef std::map<TIndexType, TCacheObject> TIndexMap;
};


template <class TDataType> class CDataCache
{
public:
	typedef CCacheObject<TDataType> TCacheObject;
//	typedef template<class TIndexType, class TDataType> class CCacheIndex
	typedef std::map<std::string, TCacheObject> TCacheIndex;

public:
	CDataCache(void);
	virtual ~CDataCache(void);

};
