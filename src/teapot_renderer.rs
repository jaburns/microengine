use glium;
use glium::Surface;
use image;
use math_ext;
use std::io::Cursor;
use teapot;
use transform::Transform;

pub struct TeapotRenderer {
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

    pub fn draw(
        &self,
        target: &mut glium::Frame,
        perspective: &[[f32; 4]; 4],
        transform: &Transform,
    ) {
        let params = glium::DrawParameters {
            depth: glium::Depth {
                test: glium::draw_parameters::DepthTest::IfLess,
                write: true,
                ..Default::default()
            },
            ..Default::default()
        };

        let uniforms = uniform! {
            perspective: *perspective,
            matrix: math_ext::matrix4_to_uniform(&transform.world_matrix),
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
            ).unwrap();
    }
}
