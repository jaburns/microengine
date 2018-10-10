#include <functional>
#include <vector>
#include <cstdint>
#include <optional>
#include <tuple>

using std::vector;
using std::optional;
using std::tuple;
using std::reference_wrapper;

struct GenerationalIndex
{
    uint32_t index = 0;
    uint32_t generation = 0;
};

class GenerationalIndexAllocator
{
    struct AllocatorEntry
    {
        bool is_live = false;
        uint32_t generation = 0;
    };

    vector<AllocatorEntry> m_entries;
    vector<uint32_t> m_free_indices;

public:

    GenerationalIndex allocate();
    void deallocate(GenerationalIndex index);
    bool is_live(GenerationalIndex index) const;
};

GenerationalIndex GenerationalIndexAllocator::allocate()
{
    if (m_free_indices.size() > 0)
    {
        uint32_t index = m_free_indices.back();
        m_free_indices.pop_back();

        m_entries[index].generation += 1;
        m_entries[index].is_live = true;

        return { index, m_entries[index].generation };
    }
    else
    {
        m_entries.push_back({ true, 0 });
        return { static_cast<uint32_t>(m_entries.size()) - 1, 0 };
    }
}

void GenerationalIndexAllocator::deallocate(GenerationalIndex index)
{
    if (is_live(index))
    {
        m_entries[index.index].is_live = false;
        m_free_indices.push_back(index.index);
    }
}

bool GenerationalIndexAllocator::is_live(GenerationalIndex index) const
{
    return index.index < m_entries.size() &&
        m_entries[index.index].generation == index.generation &&
        m_entries[index.index].is_live;
}

template<typename T>
class GenerationalIndexArray
{
    struct Entry
    {
        uint32_t generation;
        T value;
    };

    vector<optional<Entry>> m_entries;

public:

    void set(GenerationalIndex index, T value)
    {
        while (m_entries.size() <= index.index)
            m_entries.push_back(std::nullopt);

        uint32_t prev_gen = 0;

        if (auto prev_entry = m_entries[index.index]) 
            prev_gen = prev_entry->generation;

        if (prev_gen > index.generation)
            exit(1);

        m_entries[index.index] = optional<Entry>{{ index.generation, value }};
    }

    void remove(GenerationalIndex index) 
    {
        if (index.index < m_entries.size())
            m_entries[index.index] = std::nullopt;
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

#include <iostream>


int ecs_test() {
    using namespace std;
    
    GenerationalIndexArray<float> floaties;
    GenerationalIndexAllocator alloc;

    auto j = alloc.allocate();
    auto i = alloc.allocate();

    floaties.set(i, 2.0f);
    floaties.set(j, 4.0f);
    
    if (auto getty = floaties.get_first_valid_entry(alloc)) {
        cout << "Got " << std::get<1>(*getty).get();
    }

    return 0;
}


