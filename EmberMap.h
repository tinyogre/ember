
#ifndef EMBER_MAP_H__
#define EMBER_MAP_H__

#include "EmberCtx.h"
#include "EmberString.h"
#include "EmberMacros.h"

#include <strings.h>

template <class T>
class SimpleStringMapNode
{
private:
    char *m_key;
    T *m_next;
public:
    SimpleStringMapNode(EmberCtx *ctx, const char *key) {
        m_key = ctx->StrDup(key);
    }

    const char *GetKey(StringMap<T> *map) {
        return m_key;
    }

    T *GetNext(StringMap<T> *map) {
        return m_next;
    }

    void SetNext(StringMap<T> *map, T *next) {
        m_next = next;
    }
};

template <class T>
class StringMap {
private:
    T **m_buckets;
    int m_numBuckets;
    EmberCtx *m_ctx;
    unsigned int m_mask;

    int GetBucket(const char *name) {
        unsigned int hashval = ember::hash((const unsigned char *)name);
        return hashval & m_mask;
    }

public:
    struct compare {
        bool operator()(const char *k1, const char *k2) const
        {
            return strcasecmp(k1, k2) == 0;
        }
    };

    class iterator {
        StringMap<T> *m_map;
        int m_currBucket;
        T *m_currObj;
        T *m_next;

    public:
        iterator(StringMap<T> *map) :
            m_map(map),
            m_currBucket(-1),
            m_currObj(NULL),
            m_next(NULL)
        {
        }

        T *First() {
            for(m_currBucket = 0; m_currBucket < m_map->m_numBuckets; m_currBucket++) {
                m_currObj = m_map->m_buckets[m_currBucket];
                if(m_currObj) {
                    m_next = m_currObj->GetNext(m_map);
                    return m_currObj;
                }
            }
            return NULL;
        }

        T *Next() {
            if(m_next) {
                m_currObj = m_next;
                m_next = m_currObj->GetNext(m_map);
                return m_currObj;
            }
            for(++m_currBucket; m_currBucket < m_map->m_numBuckets; ++m_currBucket) {
                m_currObj = m_map->m_buckets[m_currBucket];
                if(m_currObj) {
                    m_next = m_currObj->GetNext(m_map);
                    return m_currObj;
                }
            }
            return NULL;
        }

        T *Current() {
            return m_currObj;
        }
        
        T *operator ->() { return m_currObj; }
    };

    StringMap(EmberCtx *ctx, unsigned int initialBuckets = 32) {
        m_ctx = ctx;
        m_numBuckets = initialBuckets;

        // m_numBuckets must be a power of 2
        embASSERT(m_numBuckets != 0);
        embASSERT((m_numBuckets & (m_numBuckets - 1)) == 0);

        m_buckets = (T **)m_ctx->Alloc(sizeof(T *) * m_numBuckets);
        memset(m_buckets, 0, sizeof(T *) * m_numBuckets);
        m_mask = m_numBuckets - 1;
    }

    ~StringMap()
    {
        m_ctx->Free(m_buckets);
    }

    T *Find(const char *name) {
        int bucket = GetBucket(name);
        compare c;
        for(T *obj = m_buckets[bucket]; obj; obj = obj->GetNext(this)) {
            if(c(name, obj->GetKey(this))) {
                return obj;
            }
        }
        return NULL;
    }

    bool Insert(T *obj) {
        const char *name = obj->GetKey(this);
        T *existing = Find(name);
        if(existing) {
            return false;
        }
        int bucket = GetBucket(name);
        obj->SetNext(this, m_buckets[bucket]);
        m_buckets[bucket] = obj;
        return true;
    }

    bool Remove(T *obj)
    {
        const char *name = obj->GetKey(this);
        int bucket = GetBucket(name);
        compare c;
        T *prev = NULL;
        for(T *chk = m_buckets[bucket]; chk; chk = chk->GetNext(this)) {
            if(chk == obj) {
                if(prev) {
                    prev->SetNext(this, chk->GetNext(this));
                    chk->SetNext(this, NULL);
                } else {
                    m_buckets[bucket] = chk->GetNext(this);
                }
                return true;
            }
        }
        return false;
    }

    T *Remove(const char *name)
    {
        T *obj = Find(name);
        if(obj) {
            Remove(obj);
        }
        return obj;
    }

    void Remove(iterator &it)
    {
        Remove(it.Current());
    }
};

#endif
