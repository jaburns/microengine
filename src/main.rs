#[macro_use]
extern crate glium;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;

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

fn get_perspective_matrix(screen_dimensions: (u32, u32)) -> [[f32; 4]; 4] {
    let (width, height) = screen_dimensions;
    let aspect_ratio = height as f32 / width as f32;

    let fov: f32 = 3.141592 / 3.0;
    let zfar = 1024.0;
    let znear = 0.1;
    let f = 1.0 / (fov / 2.0).tan();

    [
        [f * aspect_ratio, 0.0, 0.0, 0.0],
        [0.0, f, 0.0, 0.0],
        [0.0, 0.0, (zfar + znear) / (zfar - znear), 1.0],
        [0.0, 0.0, -(2.0 * zfar * znear) / (zfar - znear), 0.0],
    ]
}

fn run_ui(ui: &Ui, state: &mut f32) {
    ui.window(im_str!("Hello world"))
        .size((300.0, 100.0), ImGuiCond::FirstUseEver)
        .build(|| {
            ui.text(im_str!("Hello world!"));
            ui.text(im_str!("This...is...imgui-rs!"));
            SliderFloat::new(ui, im_str!("Slider"), state, 0f32, 10f32).build();

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

    let mut t: f32 = 0.0;
    let mut closed = false;

    while !closed {
        t += 0.0008;
        if t > 2.0 * 3.14159 {
            t -= 2.0 * 3.14159;
        }

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
        let perspective = get_perspective_matrix(target.get_dimensions());
        teapot.draw(&mut target, perspective, t);
        imgui_renderer.draw(&display, &mut target, &run_ui, &mut t);
        target.finish().unwrap();
    }
}
