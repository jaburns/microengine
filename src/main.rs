#[macro_use]
extern crate glium;
extern crate anymap;
extern crate cgmath;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;

mod ecs;
mod imgui_renderer;
mod math_ext;
mod render_system;
mod teapot;
mod teapot_renderer;
mod transform;

use ecs::ECS;
use render_system::RenderSystem;
use transform::{transform_system,Transform};

fn main() {
    let mut ecs = ECS::new();
    let mut render_system = RenderSystem::new(&mut ecs);
    let mut closed = false;

    let teapot = ecs.create_entity();
    let mut trans = Transform::new(0f32, 0f32, 2f32, 0.01f32);

    let teaparent = ecs.create_entity();
    ecs.set_component(teaparent, Transform::default());

    trans.parent = Some(teaparent);
    ecs.set_component(teapot, trans);

    while !closed {
        transform_system(&mut ecs);
        closed = !render_system.run(&mut ecs);
    }
}
