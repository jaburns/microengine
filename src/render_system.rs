use ecs::ECS;
use glium;
use glium::{glutin, Surface};
use imgui::*;
use imgui_renderer::ImguiRenderer;
use math_ext;
use teapot_renderer::TeapotRenderer;
use transform::Transform;
use cgmath::*;

fn run_editor(ui: &Ui, ecs: &mut ECS) {
    ui.window(im_str!("Editor"))
        .size((300.0, 300.0), ImGuiCond::FirstUseEver)
        .build(|| {
            let mut i = 0i32;
            for e in ecs.find_all_entities_with_component::<Transform>() {
                let mut t = ecs.get_component_mut::<Transform>(e).unwrap();

                ui.tree_node(im_str!("Transform {}", i)).build(|| {
                    let mut euler: Euler<Rad<f32>>;

                    SliderFloat::new(ui, im_str!("x"), &mut t.position.x, -10f32, 10f32).build();
                    SliderFloat::new(ui, im_str!("y"), &mut t.position.y, -10f32, 10f32).build();
                    SliderFloat::new(ui, im_str!("z"), &mut t.position.z, -10f32, 20f32).build();

                    euler = Euler::<Rad<f32>>::from(t.rotation);
                    SliderFloat::new(ui, im_str!("rx"), &mut euler.x.0, -3.14f32, 3.14f32).build();
                    t.rotation = Quaternion::<f32>::from(euler);

                    euler = Euler::<Rad<f32>>::from(t.rotation);
                    SliderFloat::new(ui, im_str!("ry"), &mut euler.y.0, -3.14f32/2f32, 3.14f32/2f32).build();
                    t.rotation = Quaternion::<f32>::from(euler);

                    euler = Euler::<Rad<f32>>::from(t.rotation);
                    SliderFloat::new(ui, im_str!("rz"), &mut euler.z.0, -3.14f32, 3.14f32).build();
                    t.rotation = Quaternion::<f32>::from(euler);
                });

                ui.separator();
                i += 1;
            }
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

        let mut target = self.display.draw();
        target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);
        let perspective = math_ext::get_perspective_matrix(target.get_dimensions());

        for e in ecs.find_all_entities_with_component::<Transform>() {
            let t = ecs.get_component::<Transform>(e).unwrap();
            self.teapot_renderer.draw(&mut target, &perspective, t);
        }

        self.imgui_renderer
            .draw(&self.display, &mut target, &run_editor, ecs);
        target.finish().unwrap();

        !closed
    }
}
