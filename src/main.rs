#[macro_use]
extern crate glium;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;
extern crate cgmath;

mod imgui_renderer;
mod teapot;
mod teapot_renderer;

use glium::{glutin, Surface};
use imgui::*;
use imgui_renderer::ImguiRenderer;
use teapot_renderer::TeapotRenderer;

#[derive(Copy, Clone)]
struct Vertex {
    position: [f32; 3],
}
implement_vertex!(Vertex, position);

fn run_ui(ui: &Ui, state: &mut (f32,f32,f32)) {
    ui.window(im_str!("Hello world"))
        .size((300.0, 100.0), ImGuiCond::FirstUseEver)
        .build(|| {
            ui.text(im_str!("Hello world!"));
            ui.text(im_str!("This...is...imgui-rs!"));
            SliderFloat::new(ui, im_str!("x"), &mut state.0, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("y"), &mut state.1, -10f32, 10f32).build();
            SliderFloat::new(ui, im_str!("z"), &mut state.2, -10f32, 10f32).build();

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

    let mut t  = (0.0f32, 0.0f32, 0.0f32);
    let mut closed = false;

    while !closed {
    //  t.0 += 0.0008;
    //  if t.0 > 2.0 * 3.14159 {
    //      t.0 -= 2.0 * 3.14159;
    //  }

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

        let (width, height) = target.get_dimensions();
        let aspect_ratio = width as f32 / height as f32;
    //  let perspective = cgmath::perspective(cgmath::Rad(3.14159f32 / 3.0f32), aspect_ratio, 0.1f32, 1024.0f32);
    //  let perspectiveMat = teapot_renderer::matrix4_to_uniform(&perspective);

        let perspective2 = teapot_renderer::get_perspective_matrix(target.get_dimensions());

        teapot.draw(&mut target, &perspective2, t);

        imgui_renderer.draw(&display, &mut target, &run_ui, &mut t);
        target.finish().unwrap();
    }
}
