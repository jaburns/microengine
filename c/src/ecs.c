#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "vec.h"

typedef struct GenerationalIndex 
{
    uint32_t generation;
    uint32_t index;
}
GenerationalIndex;

typedef struct AllocatorEntry
{
    bool is_live;
    uint32_t generation;
}
AllocatorEntry;

typedef struct GenerationalIndexAllocator
{
    Vec entries; // of AllocatorEntry
    Vec free_indices; // of uint32_t
}
GenerationalIndexAllocator;

GenerationalIndexAllocator giallocator_new()
{
    return (GenerationalIndexAllocator) { vec_new(sizeof(AllocatorEntry)), vec_new(sizeof(uint32_t)) };
}

GenerationalIndex giallocator_allocate(GenerationalIndexAllocator *gia)
{
    if (gia->entries.item_count > 0)
    {
        uint32_t index;
        vec_pop(&gia->entries, &index);

        AllocatorEntry *entry = vec_get(&gia->entries, index);
        entry->generation += 1;
        entry->is_live = true;

        return (GenerationalIndex) { entry->generation, index };
    }

    AllocatorEntry new_entry = (AllocatorEntry) { true, 0 };
    vec_push(&gia->entries, &new_entry);

    return (GenerationalIndex) { gia->entries.item_count - 1, 0 };
}

void giallocator_deallocate(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (! giallocator_is_index_live(gia, index)) return;

    AllocatorEntry *entry = vec_get(&gia->entries, index.index);
    entry->is_live = false;

    uint32_t x = index.index;
    vec_push(&gia->free_indices, &x);
}

bool giallocator_is_index_live(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return false;

    AllocatorEntry *entry = vec_get(&gia->entries, index.index);
    
    return entry->is_live && entry->generation == index.generation;
}

typedef struct GenerationalIndexArrayEntry
{
    bool has_value;
    uint32_t generation;
    uint8_t entry[];
}
GenerationalIndexArrayEntry;


typedef struct GenerationalIndexArray
{
    size_t item_size;

    Vec entries; // of (GenerationalIndexArrayEntryHeader + item type)
}
GenerationalIndexArray;

GenerationalIndexArray giarray_new(size_t item_size)
{
    return (GenerationalIndexArray) { item_size, vec_new(sizeof(GenerationalIndexArrayEntry) + item_size) };
}

/*

void giarray_set(GenerationalIndexArray *gia, GenerationalIndex index, void *value)
{
    while (gia->entries.item_count < index.index)
    {
        GenerationalIndexArrayEntry empty_header = (GenerationalIndexArrayEntry) { false, 0 };



        vec_push()
        gia->entries
    }

    while (m_entries.size() <= index.index)
        m_entries.push_back(std::nullopt);

    uint32_t prev_gen = 0;

    if (auto prev_entry = m_entries[index.index]) 
        prev_gen = prev_entry->generation;

    if (prev_gen > index.generation)
        exit(1);

    m_entries[index.index] = optional<Entry>{{ index.generation, value }};
}

/*
void giarray_remove(GenerationalIndexArray *gia, GenerationalIndex index)
{
    if (index.index < m_entries.size())
        m_entries[index.index] = std::nullopt;
}

    void remove(GenerationalIndex index) 
    {
    }

public:

    T* get(GenerationalIndex index)
    {
        if (index.index >= m_entries.size()) return nullptr;

        if (auto& entry = m_entries[index.index]) 
        {
            if (entry->generation == index.generation)
                return &entry->value;
        }

        return nullptr;
    }

    const T* get(GenerationalIndex index) const
    {
        return const_cast<const T*>(const_cast<GenerationalIndexArray*>(this)->get(index));
    }

    vector<GenerationalIndex> get_all_valid_indices(const GenerationalIndexAllocator& allocator) const
    {
        vector<GenerationalIndex> result;
        
        for (auto i = 0; i < m_entries.size(); ++i)
        {
            const auto& entry = m_entries[i];
            if (!entry) continue;

            GenerationalIndex index = { i, entry->generation };
            
            if (allocator.is_live(index))
                result.push_back(index);
        }

        return result;
    }

    optional<tuple<GenerationalIndex, reference_wrapper<const T>>> get_first_valid_entry(const GenerationalIndexAllocator& allocator) const
    {
        for (auto i = 0; i < m_entries.size(); ++i)
        {
            const auto& entry = m_entries[i];
            if (!entry) continue;

            GenerationalIndex index = { i, entry->generation };
            
            if (allocator.is_live(index))
                return std::make_tuple(index, std::ref(entry->value));
        }

        return std::nullopt;
    }
};
*/
