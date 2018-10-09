#[macro_use]
extern crate glium;
extern crate cgmath;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;
extern crate anymap;

mod imgui_renderer;
mod math_ext;
mod teapot;
mod teapot_renderer;
mod ecs;
mod render_system;

use ecs::ECS;
use render_system::RenderSystem;

fn main() {
    let mut ecs = ECS::new();
    let mut render_system = RenderSystem::new(&mut ecs);
    let mut closed = false;

    while !closed {
        closed = !render_system.run(&mut ecs);
    }
}
