use anymap::AnyMap;

#[derive(Copy,Clone,Eq,PartialEq)]
struct GenerationalIndex {
    index: usize,
    generation: usize,
}

struct AllocatorEntry {
    is_live: bool,
    generation: usize,
}

struct GenerationalIndexAllocator {
    entries: Vec<AllocatorEntry>,
    free: Vec<usize>,
}

impl GenerationalIndexAllocator {
    fn new() -> GenerationalIndexAllocator {
        GenerationalIndexAllocator {
            entries: Vec::new(),
            free: Vec::new(),
        }
    }

    fn allocate(&mut self) -> GenerationalIndex { 
        match self.free.pop() {
            Some(index) => {
                self.entries[index].generation += 1;
                self.entries[index].is_live = true;

                GenerationalIndex {
                    index,
                    generation: self.entries[index].generation
                }
            },
            None => {
                self.entries.push(AllocatorEntry {
                    is_live: true,
                    generation: 0
                });

                GenerationalIndex {
                    index: self.entries.len() - 1,
                    generation: 0,
                }
            }
        }
    }

    fn deallocate(&mut self, index: GenerationalIndex) -> bool {
        if self.is_live(index) {
            self.entries[index.index].is_live = false;
            self.free.push(index.index);
            true
        } else {
            false
        }
    }
    
    fn is_live(&self, index: GenerationalIndex) -> bool { 
        index.index < self.entries.len() 
            && self.entries[index.index].generation == index.generation 
            && self.entries[index.index].is_live
    }
}

#[test]
fn generational_index_allocator_works() {
    let mut alloc = GenerationalIndexAllocator::new();

    let i = alloc.allocate();
    let j = alloc.allocate();

    assert_eq!(i.generation == 0, i.index == 0);
    assert_eq!(j.generation == 0, j.index == 1);

    assert_eq!(alloc.deallocate(i), true);
    assert_eq!(alloc.deallocate(i), false);

    let k = alloc.allocate();

    assert_eq!(k.generation == 1, k.index == 0);

    assert_eq!(alloc.is_live(i), false);
    assert_eq!(alloc.is_live(j), true);
    assert_eq!(alloc.is_live(k), true);
}

struct ArrayEntry<T> {
    value: T,
    generation: usize,
}

struct GenerationalIndexArray<T>(Vec<Option<ArrayEntry<T>>>);

impl<T> GenerationalIndexArray<T> {
    fn new() -> GenerationalIndexArray<T> {
        GenerationalIndexArray(Vec::new())
    }

    fn set(&mut self, index: GenerationalIndex, value: T) {
        while self.0.len() <= index.index {
            self.0.push(None);
        }

        let prev_gen = match &self.0[index.index] {
            Some(entry) => entry.generation,
            None => 0
        };

        if prev_gen > index.generation {
            panic!("Attempted to write to GenerationalIndexArray with an index from previous generation");
        }

        self.0[index.index] = Some(ArrayEntry {
            value,
            generation: index.generation
        });
    }

    fn remove(&mut self, index: GenerationalIndex) {
        if index.index < self.0.len() {
            self.0[index.index] = None;
        }
    }

    fn get(&self, index: GenerationalIndex) -> Option<&T> {
        if index.index >= self.0.len() {
            return None;
        }

        match &self.0[index.index] {
            Some(entry) => if entry.generation == index.generation {
                    Some(&entry.value)
                } else {
                    None
                },
            None => None
        }
    }

    fn get_mut(&mut self, index: GenerationalIndex) -> Option<&mut T> { 
        if index.index >= self.0.len() {
            return None;
        }

        match &mut self.0[index.index] {
            Some(entry) => if entry.generation == index.generation {
                    Some(&mut entry.value)
                } else {
                    None
                },
            None => None
        }
    }

    fn get_all_valid(&self, allocator: &GenerationalIndexAllocator) -> Vec<(GenerationalIndex, &T)> {
        let mut result = Vec::new();

        for i in 0..self.0.len() {
            if let Some(entry) = &self.0[i] {
                let index = GenerationalIndex { index: i, generation: entry.generation };
                if allocator.is_live(index) {
                    result.push((index, &entry.value));
                }
            }
        }

        result
    }
}

#[test]
#[should_panic]
fn generational_index_array_prev_generation_access_panics() {
    let mut alloc = GenerationalIndexAllocator::new();
    let mut arr = GenerationalIndexArray::<()>::new();

    let i = alloc.allocate();
    arr.set(i, ());
    alloc.deallocate(i);
    let j = alloc.allocate();
    arr.set(j, ());
    arr.set(i, ());
}

#[test]
fn generational_index_array_works_with_multi_gen() {
    let mut alloc = GenerationalIndexAllocator::new();
    let mut arr = GenerationalIndexArray::<i32>::new();

    let i = alloc.allocate();
    arr.set(i, 1);

    match arr.get(i) {
        Some(&x) => assert_eq!(x, 1),
        None => assert!(false),
    }

    alloc.deallocate(i);

    let j = alloc.allocate();
    arr.set(j, 2);

    assert!(arr.get(i).is_none());

    match arr.get(j) {
        Some(&x) => assert_eq!(x, 2),
        None => assert!(false),
    }
}

#[derive(Copy,Clone,Eq,PartialEq)]
pub struct Entity(GenerationalIndex);

pub struct ECS {
    entity_allocator: GenerationalIndexAllocator,
    entity_components: AnyMap,
}

impl ECS {
    pub fn new() -> ECS {
        ECS {
            entity_allocator: GenerationalIndexAllocator::new(),
            entity_components: AnyMap::new(),
        }
    }

    pub fn new_entity(&mut self) -> Entity {
        Entity(self.entity_allocator.allocate())
    }

    pub fn destroy_entity(&mut self, entity: Entity) {
        self.entity_allocator.deallocate(entity.0);
    }

    pub fn is_entity_valid(&self, entity: Entity) -> bool {
        self.entity_allocator.is_live(entity.0)
    }

    pub fn set_component<T: 'static>(&mut self, entity: Entity, value: T) -> &T {
        if !self.entity_allocator.is_live(entity.0) {
            panic!("Attempted to set_component on invalid entity.")
        }

        if !self.entity_components.contains::<GenerationalIndexArray<T>>() {
            self.entity_components.insert(GenerationalIndexArray::<T>::new());
        }

        let map = self.entity_components.get_mut::<GenerationalIndexArray<T>>().unwrap();
        map.set(entity.0, value);
        map.get(entity.0).unwrap()
    }

    pub fn get_component<T: 'static>(&self, entity: Entity) -> Option<&T> {
        if !self.entity_allocator.is_live(entity.0) {
            panic!("Attempted to get_component on invalid entity.")
        }

        match self.entity_components.get::<GenerationalIndexArray<T>>() {
            Some(map) => map.get(entity.0),
            None => None
        }
    }

    pub fn get_component_mut<T: 'static>(&mut self, entity: Entity) -> Option<&mut T> {
        if !self.entity_allocator.is_live(entity.0) {
            panic!("Attempted to get_component_mut on invalid entity.")
        }

        match self.entity_components.get_mut::<GenerationalIndexArray<T>>() {
            Some(map) => map.get_mut(entity.0),
            None => None
        }
    }

    pub fn has_component<T: 'static>(&self, entity: Entity) -> bool {
        if !self.entity_allocator.is_live(entity.0) {
            panic!("Attempted to check has_component on invalid entity.")
        }

        self.get_component::<T>(entity).is_some()
    }

    pub fn find_all_components<T: 'static>(&self) -> Vec<(Entity, &T)> {
        match self.entity_components.get::<GenerationalIndexArray<T>>() {
            Some(map) => map.get_all_valid(&self.entity_allocator)
                .iter()
                .map(|x| (Entity(x.0), x.1))
                .collect(),
            None => Vec::new()
        }
    }

    pub fn remove_component<T: 'static>(&mut self, entity: Entity) {
        if let Some(map) = self.entity_components.get_mut::<GenerationalIndexArray<T>>() {
            map.remove(entity.0);
        }
    }
}

#[test]
#[should_panic]
fn ecs_panics_on_destroyed_entity_usage() {
    let mut ecs = ECS::new();

    let e0 = ecs.new_entity();
    ecs.set_component(e0, 1.0f32);
    ecs.destroy_entity(e0);
    ecs.get_component::<f32>(e0);
}

#[test]
fn ecs_find_all_components_works() {
    let mut ecs = ECS::new();

    let e0 = ecs.new_entity();
    let e1 = ecs.new_entity();

    ecs.set_component(e0, 1.0f32);
    ecs.set_component(e0, 1i32);
    ecs.set_component(e1, 2i32);

    for &(e, &c) in ecs.find_all_components::<i32>().iter() {
        assert!(e == e0 && c == 1i32 || e == e1 && c == 2i32);
    }
}

#[test]
fn ecs_remove_component_works() {
    let mut ecs = ECS::new();

    let e0 = ecs.new_entity();
    ecs.set_component(e0, 1.0f32);
    assert!(ecs.get_component::<f32>(e0).is_some());
    ecs.remove_component::<f32>(e0);
    assert!(ecs.get_component::<f32>(e0).is_none());
}