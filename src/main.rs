#[macro_use]
extern crate glium;
extern crate cgmath;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;

mod imgui_renderer;
mod math_ext;
mod teapot;
mod teapot_renderer;

use cgmath::Vector3;
use glium::{glutin, Surface};
use imgui::*;
use imgui_renderer::ImguiRenderer;
use teapot_renderer::TeapotRenderer;

pub struct EditorState {
    teapot_pos: Vector3<f32>,
    teapot_rot: Vector3<f32>,
}

impl EditorState {
    pub fn default() -> EditorState {
        EditorState {
            teapot_pos: Vector3::new(0.0f32, 0.0f32, 0.0f32),
            teapot_rot: Vector3::new(0.0f32, 0.0f32, 0.0f32),
        }
    }
}

fn run_ui(ui: &Ui, state: &mut EditorState) {
    ui.window(im_str!("Hello world"))
        .size((300.0, 100.0), ImGuiCond::FirstUseEver)
        .build(|| {
            ui.text(im_str!("Hello world!"));
            ui.text(im_str!("This...is...imgui-rs!"));
            SliderFloat::new(ui, im_str!("x"), &mut state.teapot_pos.x, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("y"), &mut state.teapot_pos.y, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("z"), &mut state.teapot_pos.z, -10f32, 10f32).build();

            SliderFloat::new(ui, im_str!("rx"), &mut state.teapot_rot.x, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("ry"), &mut state.teapot_rot.y, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("rz"), &mut state.teapot_rot.z, -10f32, 10f32).build();

            ui.separator();
            let mouse_pos = ui.imgui().mouse_pos();
            ui.text(im_str!(
                "Mouse Position: ({:.1},{:.1})",
                mouse_pos.0,
                mouse_pos.1
            ));
        });
}

fn main() {
    let mut events_loop = glutin::EventsLoop::new();
    let window = glutin::WindowBuilder::new();
    let context = glutin::ContextBuilder::new();
    let display = glium::Display::new(window, context, &events_loop).unwrap();

    let teapot = TeapotRenderer::new(&display);
    let mut imgui_renderer = ImguiRenderer::new(&display);

    let mut closed = false;
    let mut state = EditorState::default();

    while !closed {
        events_loop.poll_events(|event: glutin::Event| {
            use glium::glutin::Event;
            use glium::glutin::WindowEvent;

            if let Event::WindowEvent { event, .. } = event {
                if let WindowEvent::CloseRequested = event {
                    closed = true;
                }
                imgui_renderer.handle_event(&display, &event);
            }
        });

        let mut target = display.draw();
        target.clear_color_and_depth((0.0, 0.0, 1.0, 1.0), 1.0);
        let perspective = math_ext::get_perspective_matrix(target.get_dimensions());
        teapot.draw(&mut target, &perspective, &state);
        imgui_renderer.draw(&display, &mut target, &run_ui, &mut state);
        target.finish().unwrap();
    }
}
