use glium;
use glium::glutin;
use imgui;
use imgui::*;
use imgui_glium_renderer;
use imgui_glium_renderer::Renderer;
use std;
use std::time::Instant;

#[derive(Copy, Clone, PartialEq, Debug, Default)]
struct MouseState {
    pos: (i32, i32),
    pressed: (bool, bool, bool),
    wheel: f32,
}

pub struct ImguiRenderer {
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
        let renderer = Renderer::init(&mut imgui, display).expect("Failed to initialize renderer");
        configure_keys(&mut imgui);

        ImguiRenderer {
            imgui: imgui,
            renderer: renderer,
            hidpi_factor: hidpi_factor,
            mouse_state: MouseState::default(),
            last_frame: Instant::now(),
        }
    }

    pub fn draw<F, S>(
        &mut self,
        display: &glium::Display,
        frame: &mut glium::Frame,
        run_ui: F,
        ui_state: &mut S,
    ) where
        F: Fn(&Ui, &mut S),
    {
        self.imgui
            .set_mouse_pos(self.mouse_state.pos.0 as f32, self.mouse_state.pos.1 as f32);
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

        run_ui(&ui, ui_state);

        self.renderer.render(frame, ui).expect("Rendering failed");
    }

    pub fn handle_event(&mut self, display: &glium::Display, event: &glutin::WindowEvent) {
        use glium::glutin::ElementState::Pressed;
        use glium::glutin::WindowEvent::*;
        use glium::glutin::{MouseButton, MouseScrollDelta, TouchPhase};

        match event {
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
                    Some(Key::LControl) | Some(Key::RControl) => self.imgui.set_key_ctrl(pressed),
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
            _ => (),
        }
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
