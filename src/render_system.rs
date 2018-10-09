use cgmath::Vector3;
use ecs::ECS;
use glium;
use glium::{glutin, Surface};
use imgui::*;
use imgui_renderer::ImguiRenderer;
use math_ext;
use teapot_renderer::TeapotRenderer;

pub struct EditorState {
    pub teapot_pos: Vector3<f32>,
    pub teapot_rot: Vector3<f32>,
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

pub struct RenderSystem {
    events_loop: glutin::EventsLoop,
    display: glium::Display,

    teapot_renderer: TeapotRenderer,
    imgui_renderer: ImguiRenderer,
}

impl RenderSystem {
    pub fn new(ecs: &mut ECS) -> RenderSystem {
        let ent = ecs.new_entity();
        ecs.set_component(ent, EditorState::default());

        let events_loop = glutin::EventsLoop::new();
        let window = glutin::WindowBuilder::new();
        let context = glutin::ContextBuilder::new();
        let display = glium::Display::new(window, context, &events_loop).unwrap();
        let teapot_renderer = TeapotRenderer::new(&display);
        let imgui_renderer = ImguiRenderer::new(&display);

        RenderSystem {
            events_loop,
            display,
            teapot_renderer,
            imgui_renderer,
        }
    }

    pub fn run(&mut self, ecs: &mut ECS) -> bool {
        let mut closed = false;

        let mut window_events = Vec::new();

        self.events_loop.poll_events(|event: glutin::Event| {
            use glium::glutin::Event;
            use glium::glutin::WindowEvent;

            if let Event::WindowEvent { event, .. } = event {
                if let WindowEvent::CloseRequested = event {
                    closed = true;
                }
                window_events.push(event);
            }
        });

        for event in window_events.iter() {
            self.imgui_renderer.handle_event(&self.display, &event);
        }

        let mut state = ecs.find_component_mut::<EditorState>().unwrap().1;

        let mut target = self.display.draw();
        target.clear_color_and_depth((0.0, 0.0, 1.0, 1.0), 1.0);
        let perspective = math_ext::get_perspective_matrix(target.get_dimensions());
        self.teapot_renderer.draw(&mut target, &perspective, state);
        self.imgui_renderer
            .draw(&self.display, &mut target, &run_ui, &mut state);
        target.finish().unwrap();

        !closed
    }
}
