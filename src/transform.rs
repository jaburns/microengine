use cgmath::{Matrix4, One, Quaternion, SquareMatrix, Vector3};
use ecs::{Entity, ECS};

pub struct Transform {
    pub position: Vector3<f32>,
    pub rotation: Quaternion<f32>,
    pub scale: Vector3<f32>,
    pub parent: Option<Entity>,
    pub world_matrix: Matrix4<f32>,
}

impl Default for Transform {
    fn default() -> Transform {
        Transform {
            position: Vector3::new(0f32, 0f32, 0f32),
            rotation: Quaternion::one(),
            scale: Vector3::new(1f32, 1f32, 1f32),
            parent: None,
            world_matrix: Matrix4::identity(),
        }
    }
}

fn compute_local_matrix(t: &Transform) -> Matrix4<f32> {
    Matrix4::from_translation(t.position)
        * Matrix4::from(t.rotation)
        * Matrix4::from_nonuniform_scale(t.scale.x, t.scale.y, t.scale.z)
}

pub fn transform_system(ecs: &mut ECS) {
    let entities: Vec<Entity> = ecs
        .find_all_components::<Transform>()
        .iter()
        .map(|(e, _)| *e)
        .collect();

    for entity in entities {
        let mut world_matrix: Matrix4<f32>;

        {
            let t = ecs.get_component::<Transform>(entity).unwrap();
            world_matrix = compute_local_matrix(t);

            let mut parent = t.parent;
            loop {
                if let Some(entity) = parent {
                    let parent_trans = ecs.get_component::<Transform>(entity).unwrap();
                    world_matrix = world_matrix * compute_local_matrix(parent_trans);
                    parent = parent_trans.parent;
                } else {
                    break;
                }
            }
        }

        ecs.get_component_mut::<Transform>(entity)
            .unwrap()
            .world_matrix = world_matrix;
    }
}
