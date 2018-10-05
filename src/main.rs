#[macro_use]
extern crate glium;
extern crate image;
extern crate imgui;
extern crate imgui_glium_renderer;

mod teapot;

use std::io::Cursor;
use glium::{glutin, Surface};

// imgui support ===========================
use imgui::*;
use std::time::Instant;
use imgui_glium_renderer::Renderer;

#[derive(Copy, Clone, PartialEq, Debug, Default)]
struct MouseState {
    pos: (i32, i32),
    pressed: (bool, bool, bool),
    wheel: f32,
}
//==============================

#[derive(Copy, Clone)]
struct Vertex {
    position: [f32; 3],
}
implement_vertex!(Vertex, position);

fn get_perspective_matrix(screen_dimensions: (u32,u32)) -> [[f32; 4]; 4] {
    let (width, height) = screen_dimensions; //target.get_dimensions();
    let aspect_ratio = height as f32 / width as f32;

    let fov: f32 = 3.141592 / 3.0;
    let zfar = 1024.0;
    let znear = 0.1;
    let f = 1.0 / (fov / 2.0).tan();

    [
        [f * aspect_ratio, 0.0,              0.0              , 0.0],
        [       0.0      ,  f ,              0.0              , 0.0],
        [       0.0      , 0.0,      (zfar+znear)/(zfar-znear), 1.0],
        [       0.0      , 0.0, -(2.0*zfar*znear)/(zfar-znear), 0.0],
    ]
}

fn run_ui<'a>(ui: &Ui<'a>) -> bool {
    ui.window(im_str!("Hello world"))
        .size((300.0, 100.0), ImGuiCond::FirstUseEver)
        .build(|| {
            ui.text(im_str!("Hello world!"));
            ui.text(im_str!("This...is...imgui-rs!"));
            ui.separator();
            let mouse_pos = ui.imgui().mouse_pos();
            ui.text(im_str!(
                "Mouse Position: ({:.1},{:.1})",
                mouse_pos.0,
                mouse_pos.1
            ));
        });

    true
}

struct TeapotRenderer {
    positions: glium::VertexBuffer<teapot::Vertex>,
    normals: glium::VertexBuffer<teapot::Normal>,
    indices: glium::IndexBuffer<u16>,
    program: glium::Program,
    texture: glium::Texture2d,
}

impl TeapotRenderer {
    pub fn new(display: &glium::Display) -> TeapotRenderer {
        let positions = glium::VertexBuffer::new(display, &teapot::VERTICES).unwrap();
        let normals = glium::VertexBuffer::new(display, &teapot::NORMALS).unwrap();
        let indices = glium::IndexBuffer::new(display, glium::index::PrimitiveType::TrianglesList, &teapot::INDICES).unwrap();
        let program = glium::Program::from_source(display, include_str!("../res/test.vert"), include_str!("../res/test.frag"), None).unwrap();

        let image = image::load(Cursor::new(&include_bytes!("../res/texture.png")[..]), image::PNG).unwrap().to_rgba();
        let image_dimensions = image.dimensions();
        let image = glium::texture::RawImage2d::from_raw_rgba_reversed(&image.into_raw(), image_dimensions);
        let texture = glium::texture::Texture2d::new(display, image).unwrap();

        TeapotRenderer {
            positions: positions,
            normals: normals,
            indices: indices,
            program: program,
            texture: texture
        }
    }

    pub fn draw(&self, target: &mut glium::Frame, perspective: [[f32; 4]; 4], t: f32) {
        let params = glium::DrawParameters {
            depth: glium::Depth {
                test: glium::draw_parameters::DepthTest::IfLess,
                write: true,
                .. Default::default()
            },
            backface_culling: glium::draw_parameters::BackfaceCullingMode::CullClockwise,
            .. Default::default()
        };

        let uniforms = uniform! {
            perspective: perspective,
            matrix: [
                [0.01 * t.cos(), 0.0, 0.01 * t.sin(), 0.0],
                [0.0, 0.01, 0.0, 0.0],
                [0.01 *-t.sin(), 0.0, 0.01 * t.cos(), 0.0],
                [0.0, 0.0, 2.0, 1.0f32],
            ],
            light: [0.005*t.sin(), 0.4, 0.9f32],
            tex: &self.texture,
        };

        target.draw((&self.positions, &self.normals), &self.indices, &self.program, &uniforms, &params).unwrap();
    }
}

struct ImguiRenderer {
    imgui: imgui::ImGui,
    renderer: imgui_glium_renderer::Renderer,
    hidpi_factor: f64,
    mouse_state: MouseState,
    last_frame: std::time::Instant,
}

impl ImguiRenderer {
    pub fn new(display: &glium::Display) -> ImguiRenderer {
        let mut imgui = ImGui::init();
        imgui.set_ini_filename(None);
        let window = display.gl_window();
        let hidpi_factor = window.get_hidpi_factor().round();
        imgui.set_font_global_scale((1.0 / hidpi_factor) as f32);
        let mut renderer = Renderer::init(&mut imgui, display).expect("Failed to initialize renderer");
        configure_keys(&mut imgui);

        ImguiRenderer {
            imgui: imgui,
            renderer: renderer,
            hidpi_factor: hidpi_factor,
            mouse_state: MouseState::default(),
            last_frame: Instant::now(),
        }
    }

    pub fn draw(&mut self, display: &glium::Display, frame: &mut glium::Frame) {
        self.imgui.set_mouse_pos(self.mouse_state.pos.0 as f32, self.mouse_state.pos.1 as f32);
        self.imgui.set_mouse_down([
            self.mouse_state.pressed.0,
            self.mouse_state.pressed.1,
            self.mouse_state.pressed.2,
            false,
            false,
        ]);
        self.imgui.set_mouse_wheel(self.mouse_state.wheel);
        self.mouse_state.wheel = 0.0;

        let now = Instant::now();
        let delta = now - self.last_frame;
        let delta_s = delta.as_secs() as f32 + delta.subsec_nanos() as f32 / 1_000_000_000.0;
        self.last_frame = now;

        let window = display.gl_window();

        let mouse_cursor = self.imgui.mouse_cursor();
        if self.imgui.mouse_draw_cursor() || mouse_cursor == ImGuiMouseCursor::None {
            window.hide_cursor(true);
        } else {
            window.hide_cursor(false);
            window.set_cursor(match mouse_cursor {
                ImGuiMouseCursor::None => unreachable!("mouse_cursor was None!"),
                ImGuiMouseCursor::Arrow => glutin::MouseCursor::Arrow,
                ImGuiMouseCursor::TextInput => glutin::MouseCursor::Text,
                ImGuiMouseCursor::Move => glutin::MouseCursor::Move,
                ImGuiMouseCursor::ResizeNS => glutin::MouseCursor::NsResize,
                ImGuiMouseCursor::ResizeEW => glutin::MouseCursor::EwResize,
                ImGuiMouseCursor::ResizeNESW => glutin::MouseCursor::NeswResize,
                ImGuiMouseCursor::ResizeNWSE => glutin::MouseCursor::NwseResize,
            });
        }

        let physical_size = window
            .get_inner_size()
            .unwrap()
            .to_physical(window.get_hidpi_factor());
        let logical_size = physical_size.to_logical(self.hidpi_factor);

        let frame_size = FrameSize {
            logical_size: logical_size.into(),
            hidpi_factor: self.hidpi_factor,
        };

        let ui = self.imgui.frame(frame_size, delta_s);
        if !run_ui(&ui) {
            panic!("Shit, break");
        }

        self.renderer.render(frame, ui).expect("Rendering failed");
    }

 //   pub fn draw2(&self, display: &mut glium::Display) {
 //   }

    pub fn handle_event(&mut self, display: &glium::Display, event: &glutin::Event) {
        use glium::glutin::ElementState::Pressed;
        use glium::glutin::WindowEvent::*;
        use glium::glutin::{Event, MouseButton, MouseScrollDelta, TouchPhase};

        match event {
            Event::WindowEvent { event, .. } => match event {
                    KeyboardInput { input, .. } => {
                    use glium::glutin::VirtualKeyCode as Key;

                    let pressed = input.state == Pressed;
                    match input.virtual_keycode {
                        Some(Key::Tab) => self.imgui.set_key(0, pressed),
                        Some(Key::Left) => self.imgui.set_key(1, pressed),
                        Some(Key::Right) => self.imgui.set_key(2, pressed),
                        Some(Key::Up) => self.imgui.set_key(3, pressed),
                        Some(Key::Down) => self.imgui.set_key(4, pressed),
                        Some(Key::PageUp) => self.imgui.set_key(5, pressed),
                        Some(Key::PageDown) => self.imgui.set_key(6, pressed),
                        Some(Key::Home) => self.imgui.set_key(7, pressed),
                        Some(Key::End) => self.imgui.set_key(8, pressed),
                        Some(Key::Delete) => self.imgui.set_key(9, pressed),
                        Some(Key::Back) => self.imgui.set_key(10, pressed),
                        Some(Key::Return) => self.imgui.set_key(11, pressed),
                        Some(Key::Escape) => self.imgui.set_key(12, pressed),
                        Some(Key::A) => self.imgui.set_key(13, pressed),
                        Some(Key::C) => self.imgui.set_key(14, pressed),
                        Some(Key::V) => self.imgui.set_key(15, pressed),
                        Some(Key::X) => self.imgui.set_key(16, pressed),
                        Some(Key::Y) => self.imgui.set_key(17, pressed),
                        Some(Key::Z) => self.imgui.set_key(18, pressed),
                        Some(Key::LControl) | Some(Key::RControl) => {
                            self.imgui.set_key_ctrl(pressed)
                        }
                        Some(Key::LShift) | Some(Key::RShift) => self.imgui.set_key_shift(pressed),
                        Some(Key::LAlt) | Some(Key::RAlt) => self.imgui.set_key_alt(pressed),
                        Some(Key::LWin) | Some(Key::RWin) => self.imgui.set_key_super(pressed),
                        _ => {}
                    }
                }
                CursorMoved { position: pos, .. } => {
                    self.mouse_state.pos = pos
                        .to_physical(display.gl_window().get_hidpi_factor())
                        .to_logical(self.hidpi_factor)
                        .into();
                }
                MouseInput { state, button, .. } => match button {
                    MouseButton::Left => self.mouse_state.pressed.0 = *state == Pressed,
                    MouseButton::Right => self.mouse_state.pressed.1 = *state == Pressed,
                    MouseButton::Middle => self.mouse_state.pressed.2 = *state == Pressed,
                    _ => {}
                },
                MouseWheel {
                    delta: MouseScrollDelta::LineDelta(_, y),
                    phase: TouchPhase::Moved,
                    ..
                } => self.mouse_state.wheel = *y,
                MouseWheel {
                    delta: MouseScrollDelta::PixelDelta(pos),
                    phase: TouchPhase::Moved,
                    ..
                } => {
                    self.mouse_state.wheel = pos
                        .to_physical(display.gl_window().get_hidpi_factor())
                        .to_logical(self.hidpi_factor)
                        .y as f32;
                }
                ReceivedCharacter(c) => self.imgui.add_input_character(*c),
                _ => ()
            },
            _ => (),
        }
    }
}

fn main() {
    let mut events_loop = glutin::EventsLoop::new();
    let window = glutin::WindowBuilder::new();
    let context = glutin::ContextBuilder::new();
    let display = glium::Display::new(window, context, &events_loop).unwrap();

    let teapot = TeapotRenderer::new(&display);
    let mut imgui_renderer = ImguiRenderer::new(&display);

    let mut t: f32 = -0.5;
    let mut closed = false;

    while !closed {
        t += 0.0008;
        if t > 3.14159 {
            t = -3.14159;
        }

        events_loop.poll_events(|event: glutin::Event| {
            use glium::glutin::WindowEvent::*;
            use glium::glutin::Event;

            imgui_renderer.handle_event(&display, &event);

            if let Event::WindowEvent { event, .. } = event {
                match event {
                    CloseRequested => closed = true,
                    _ => {}
                }
            }
        });


        let mut target = display.draw();
        target.clear_color_and_depth((0.0, 0.0, 1.0, 1.0), 1.0);
        let perspective = get_perspective_matrix(target.get_dimensions());
        teapot.draw(&mut target, perspective, t);

        imgui_renderer.draw(&display, &mut target);

        //imgui_renderer.draw2(&mut display);

        target.finish().unwrap();
    }
}

fn configure_keys(imgui: &mut ImGui) {
    use imgui::ImGuiKey;

    imgui.set_imgui_key(ImGuiKey::Tab, 0);
    imgui.set_imgui_key(ImGuiKey::LeftArrow, 1);
    imgui.set_imgui_key(ImGuiKey::RightArrow, 2);
    imgui.set_imgui_key(ImGuiKey::UpArrow, 3);
    imgui.set_imgui_key(ImGuiKey::DownArrow, 4);
    imgui.set_imgui_key(ImGuiKey::PageUp, 5);
    imgui.set_imgui_key(ImGuiKey::PageDown, 6);
    imgui.set_imgui_key(ImGuiKey::Home, 7);
    imgui.set_imgui_key(ImGuiKey::End, 8);
    imgui.set_imgui_key(ImGuiKey::Delete, 9);
    imgui.set_imgui_key(ImGuiKey::Backspace, 10);
    imgui.set_imgui_key(ImGuiKey::Enter, 11);
    imgui.set_imgui_key(ImGuiKey::Escape, 12);
    imgui.set_imgui_key(ImGuiKey::A, 13);
    imgui.set_imgui_key(ImGuiKey::C, 14);
    imgui.set_imgui_key(ImGuiKey::V, 15);
    imgui.set_imgui_key(ImGuiKey::X, 16);
    imgui.set_imgui_key(ImGuiKey::Y, 17);
    imgui.set_imgui_key(ImGuiKey::Z, 18);
}
