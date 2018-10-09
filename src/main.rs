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
