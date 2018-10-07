use glium;
use glium::Surface;
use image;
use std::io::Cursor;
use teapot;
use cgmath::{Matrix,Matrix4,Rad,Vector3};

pub struct TeapotRenderer {
    positions: glium::VertexBuffer<teapot::Vertex>,
    normals: glium::VertexBuffer<teapot::Normal>,
    indices: glium::IndexBuffer<u16>,
    program: glium::Program,
    texture: glium::Texture2d,
}

pub fn get_perspective_matrix(screen_dimensions: (u32, u32)) -> [[f32; 4]; 4] {
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

pub fn matrix4_to_uniform(m0: &Matrix4<f32>) -> [[f32; 4]; 4] {
    let m = m0;
    //let m = m0.transpose();
    [
        [m.x.x, m.x.y, m.x.z, m.x.w],
        [m.y.x, m.y.y, m.y.z, m.y.w],
        [m.z.x, m.z.y, m.z.z, m.z.w],
        [m.w.x, m.w.y, m.w.z, m.w.w],
    ]
}

impl TeapotRenderer {
    pub fn new(display: &glium::Display) -> TeapotRenderer {
        let positions = glium::VertexBuffer::new(display, &teapot::VERTICES).unwrap();
        let normals = glium::VertexBuffer::new(display, &teapot::NORMALS).unwrap();
        let indices = glium::IndexBuffer::new(
            display,
            glium::index::PrimitiveType::TrianglesList,
            &teapot::INDICES,
        ).unwrap();
        let program = glium::Program::from_source(
            display,
            include_str!("../res/test.vert"),
            include_str!("../res/test.frag"),
            None,
        ).unwrap();

        let image = image::load(
            Cursor::new(&include_bytes!("../res/texture.png")[..]),
            image::PNG,
        ).unwrap()
            .to_rgba();
        let image_dimensions = image.dimensions();
        let image =
            glium::texture::RawImage2d::from_raw_rgba_reversed(&image.into_raw(), image_dimensions);
        let texture = glium::texture::Texture2d::new(display, image).unwrap();

        TeapotRenderer {
            positions: positions,
            normals: normals,
            indices: indices,
            program: program,
            texture: texture,
        }
    }

    pub fn draw(&self, target: &mut glium::Frame, perspective: &[[f32; 4]; 4], t: (f32,f32,f32)) {
        let params = glium::DrawParameters {
            depth: glium::Depth {
                test: glium::draw_parameters::DepthTest::IfLess,
                write: true,
                ..Default::default()
            },
            ..Default::default()
        };

        let model = 
            Matrix4::from_translation(Vector3::new(t.0, t.1, t.2)) * // 0.0f32, 0.0f32, 2.0f32)) *
            Matrix4::from_scale(0.01f32);

        let uniforms = uniform! {
            perspective: *perspective,
            matrix: matrix4_to_uniform(&model),
            light: [0.05f32, 0.4f32, 0.9f32],
            tex: &self.texture,
        };

        target
            .draw(
                (&self.positions, &self.normals),
                &self.indices,
                &self.program,
                &uniforms,
                &params,
            )
            .unwrap();
    }
}
