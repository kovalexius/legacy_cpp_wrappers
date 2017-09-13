#include ".\datacache.h"

/*
CDataCache<class TDataType>::CDataCache(void)
{
}

CDataCache<class TDataType>::~CDataCache(void)
{
}

// #include "../../Common/common/Variant.h"

/*
template <class T> class IDataCacheIndex
{
public:
	virtual T* GetBy(const CVariant& rVariant) = 0;
	virtual void Set(T* pData)  = 0;
};

template <class T, class T1> class CDataCacheIndex : public IDataCacheIndex<T>
{
public:
	CDataCacheIndex(size_t nOffset): m_nOffset(nOffset){}
//	~CDataCacheIndex() {}
	typedef std::map<T1, T*> TIndexMap;
	virtual T* GetBy(const CVariant& rVariant) 
	{
		TIndexMap::iterator ix = m_map.find(rVariant);
		return ix != m_map.end() ? ix->second : NULL;
	}
	virtual void Set(T* pData) 
	{
		m_map[*reinterpret_cast<T1*>(((char*)pData) + m_nOffset)] = pData;
	}

private:
	std::map<T1, T*> m_map;
	size_t m_nOffset;
};

template <class T> class CDataCache
{
public:
	void AddIndex(const std::string strIdx, IDataCacheIndex<T>* pIdx) {
		m_mapIndexes[strIdx] = pIdx;
	}
	template<class T1> T* GetBy(const std::string strIdx, const T1& rIdx)
	{
		IDataCacheIndex<T>* p = m_mapIndexes[strIdx];
		return p ? p->GetBy(rIdx) : NULL;
	}
	void Add(const T& rData) {
		T* pData = new T;
		*pData = rData;
		for (std::map<std::string, IDataCacheIndex<T>*>::iterator ix = m_mapIndexes.begin(); ix != m_mapIndexes.end(); ix++)
		{
			IDataCacheIndex<T>* pDataIndex = ix->second;
			pDataIndex->Set(pData);
		}
	}

private:
	std::map<std::string, IDataCacheIndex<T>*> m_mapIndexes;
};
*/


/*
#define INDEX(STRUCT, MEMBER) &(((STRUCT*)0)->MEMBER)

class CVariantEx
{
public:
	template <class T> CVariantEx(const T& rVal)
	{ 
		m_nVal = (T)rVal; 
	}
	CVariantEx(const std::string& str) { m_strVal = str; }
	CVariantEx(const char* pszStr) { m_strVal = pszStr; }

	template <class T> operator const T&() const { return (const T&)m_nVal; }
	operator const std::string&() const { return m_strVal; }
private:
	std::string m_strVal;
	long long m_nVal;
};

template <class T> class CDataCache
{
protected:
	template <class T> class IDataCacheIndex
	{
	public:
		virtual T* GetBy(const CVariantEx& rVariant) = 0;
		virtual void Set(T* pData)  = 0;
	};

	template <class T, class T1> class CDataCacheIndex : public IDataCacheIndex<T>
	{
	public:
		CDataCacheIndex(size_t nOffset): m_nOffset(nOffset){}
		//	~CDataCacheIndex() {}
		typedef std::map<T1, T*> TIndexMap;
		virtual T* GetBy(const CVariantEx& rVariant) 
		{
			TIndexMap::iterator ix = m_map.find(rVariant);
			return ix != m_map.end() ? ix->second : NULL;
		}
		virtual void Set(T* pData) 
		{
			m_map[*reinterpret_cast<T1*>(((char*)pData) + m_nOffset)] = pData;
		}

	private:
		std::map<T1, T*> m_map;
		size_t m_nOffset;
	};
public:
	void AddIndex(const std::string strIdx, IDataCacheIndex<T>* pIdx) {
		m_mapIndexes[strIdx] = pIdx;
	}
	template <class T1> void AddIndex(const std::string& strIdxName, size_t nOffset)
	{
		AddIndex(strIdxName, new CDataCacheIndex<T, T1>(nOffset));
	}
	template <class T1> void AddIndexEx(const std::string& strIdxName, T1* pFoo)
	{
		AddIndex(strIdxName, new CDataCacheIndex<T, T1>((size_t)pFoo));
	}
	template<class T1> T* GetBy(const std::string& strIdx, const T1& rIdx)
	{
		IDataCacheIndex<T>* p = m_mapIndexes[strIdx];
		return p ? p->GetBy(rIdx) : NULL;
	}
	void Add(const T& rData) {
		T* pData = new T;
		*pData = rData;
		for (std::map<std::string, IDataCacheIndex<T>*>::iterator ix = m_mapIndexes.begin(); ix != m_mapIndexes.end(); ix++)
		{
			IDataCacheIndex<T>* pDataIndex = ix->second;
			pDataIndex->Set(pData);
		}
	}

private:
	std::map<std::string, IDataCacheIndex<T>*> m_mapIndexes;
};


struct Q
{
	int q1;
	std::string q2;
	bool q3;
	char q4[10];
};


/////////////////////////
// тестовая версия (без CVariantEx)
template <class T> class CDataIndex2
{
	template <class T, class T1> class CDataCacheIndex
	{
	public:
		CDataCacheIndex(size_t nOffset): m_nOffset(nOffset){}
		//	~CDataCacheIndex() {}
		typedef std::map<T1, T*> TIndexMap;
		virtual T* GetBy(const CVariantEx& rVariant) 
		{
			TIndexMap::iterator ix = m_map.find(rVariant);
			return ix != m_map.end() ? ix->second : NULL;
		}
		virtual void Set(T* pData) 
		{
			m_map[*reinterpret_cast<T1*>(((char*)pData) + m_nOffset)] = pData;
		}

	private:
		std::map<T1, T*> m_map;
		size_t m_nOffset;
	};

public:
	template <class T1> void AddIndexEx(const std::string& strIdxName, T1* pFoo)
	{
		m_mapIndexes[strIdxName] = (void*)new CDataCacheIndex<T, T1>((size_t)pFoo);
	}
	template<class T1> T* GetBy(const std::string& strIdx, const T1& rIdx)
	{
		CDataCacheIndex<T, T1>* p = m_mapIndexes[strIdx];
		return p ? p->GetBy(rIdx) : NULL;
	}
	void Add(const T& rData) 
	{
		T* pData = new T;
		*pData = rData;
		for (std::map<std::string, void*>::iterator ix = m_mapIndexes.begin(); ix != m_mapIndexes.end(); ix++)
		{	//!! вот тут не катит
			IDataCacheIndex<T>* pDataIndex = ix->second;
			pDataIndex->Set(pData);
		}
	}

private:
	std::map<std::string, void*> m_mapIndexes;
};


int _tmain(int argc, _TCHAR* argv[])
{
//	A<Q, Q::q2>();
	CDataCache<Q> rCache;
//	rCache.AddIndex("q1", new CDataCacheIndex<Q, int>(offsetof(Q, q1)));
//	rCache.AddIndex("q2", new CDataCacheIndex<Q, std::string>(offsetof(Q, q2)));
//	rCache.AddIndex<int>("q1", offsetof(Q, q1));
//	rCache.AddIndex<std::string>("q2", offsetof(Q, q2));

	rCache.AddIndex<int>("q1", offsetof(Q, q1));
	rCache.AddIndexEx("q2", INDEX(Q, q2));



	Q qq1={1,"2",true,"4"}, qq2={5,"6",false,"88"}, qq3={9, "10", true, "12"}, *qq;
	rCache.Add(qq1);
	rCache.Add(qq2);
	rCache.Add(qq3);

	qq = rCache.GetBy("q1", 5);
	qq = rCache.GetBy("q1", 9);
	qq = rCache.GetBy("q1", 1);
	qq = rCache.GetBy("q2", "2");
	qq = rCache.GetBy("q2", "6");
	qq = rCache.GetBy("q2", "10");
	qq = rCache.GetBy("q2", "11");

*/