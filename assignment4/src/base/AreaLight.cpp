
#include "AreaLight.hpp"
#include "RTTriangle.hpp"


namespace FW {


void AreaLight::draw(const Mat4f& worldToCamera, const Mat4f& projection) {
    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((float*)&projection);
    glMatrixMode(GL_MODELVIEW);
    Mat4f S = Mat4f::scale(Vec3f(m_size,1));
    Mat4f M = worldToCamera *m_xform * S;
    glLoadMatrixf((float*)&M);
    glBegin(GL_TRIANGLES);
    glColor3fv( &m_E.x );
    glVertex3f(1,1,0); glVertex3f(1,-1,0); glVertex3f( -1,-1,0 );
    glVertex3f(1,1,0); glVertex3f( -1,-1,0 ); glVertex3f(-1,1,0); 
    glEnd();
}

void AreaLight::sampleEmissionTriangle(float& pdf, Vec3f& p, int base, Random& rnd, RTTriangle tri) {
    pdf = 1.0f / tri.area();

    float t1 = rnd.getF32(0, 1);
    float t2 = rnd.getF32(0, 1);
    if (t1 + t2 > 1) {
        t1 = 1 - t1;
        t2 = 1 - t2;
    }
    float t3 = 1 - t1 - t2;

    p = tri.centroid() +
        t1 * (tri.m_vertices[0].p - tri.centroid()) +
        t2 * (tri.m_vertices[1].p - tri.centroid()) +
        t3 * (tri.m_vertices[2].p - tri.centroid());
}


void AreaLight::sample(float& pdf, Vec3f& p, int base, Random& rnd) {
    // YOUR CODE HERE (R1): Integrate your area light implementation.
   
    // If doing Stratified OR Halton: 
    // Integrade these later to parameters as in assignment2
    int samplingMode = 0;
    int r = -1;
    int tableWidth = -1;

    pdf = 1 / (4 * m_size.x * m_size.y);

    // UNIFORM SAMPLING
    if (samplingMode == 0 || r == -1 || tableWidth == -1) {
        Vec2f multiplier = rnd.getVec2f(-1, 1);

        p = Vec4f(m_xform.getCol(3)).getXYZ()
            + Vec4f(m_xform.getCol(0)).getXYZ() * multiplier.x * m_size.x
            + Vec4f(m_xform.getCol(1)).getXYZ() * multiplier.y * m_size.y;
    }

    // EXTRA STRATIFIED SAMPLING
    else if (samplingMode == 2) {
        int x = r % tableWidth;
        int y = r / tableWidth;

        Vec2f multiplier = rnd.getVec2f(-1, 1);

        // Bottom-left corner of light
        p = Vec4f(m_xform.getCol(3)).getXYZ() - Vec4f(m_xform.getCol(0)).getXYZ() * m_size.x - Vec4f(m_xform.getCol(1)).getXYZ() * m_size.y;
        //Move to bottom-left corner of sub box
        p += Vec4f(m_xform.getCol(0)).getXYZ() * x * (2 * m_size.x / tableWidth) + Vec4f(m_xform.getCol(1)).getXYZ() * y * (2 * m_size.y / tableWidth);
        //Move to the center of this sub box
        p += Vec4f(m_xform.getCol(0)).getXYZ() * 0.5f * (2 * m_size.x / tableWidth) + Vec4f(m_xform.getCol(1)).getXYZ() * 0.5f * (2 * m_size.y / tableWidth);
        //Take random sample in this sub box
        p += Vec4f(m_xform.getCol(0)).getXYZ() * multiplier.x * (2 * m_size.x / tableWidth)
            + Vec4f(m_xform.getCol(1)).getXYZ() * multiplier.y * (2 * m_size.y / tableWidth);
    }

    // HALTON SAMPLING
    // SamplingMode == 1
    else {
        float multiplier_x = halton(r, 2);
        float multiplier_y = halton(r, 3);

        // Bottom-left corner of light
        p = Vec4f(m_xform.getCol(3)).getXYZ() - Vec4f(m_xform.getCol(0)).getXYZ() * m_size.x - Vec4f(m_xform.getCol(1)).getXYZ() * m_size.y;
        //Move to sampling point
        p += Vec4f(m_xform.getCol(0)).getXYZ() * 2 * m_size.x * multiplier_x + Vec4f(m_xform.getCol(1)).getXYZ() * 2 * m_size.y * multiplier_y;
    }
}

float AreaLight::halton(int index, int base) {
    float result = 0;
    float fraction = 1;
    int i = index + 1; // index can be 0, i can't

    while (i > 0) {
        fraction = fraction / base;
        result += fraction * (i % base);
        i = i / base;
    }

    return result;
}


} // namespace FW
