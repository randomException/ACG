
#include "AreaLight.hpp"


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

void AreaLight::sample(float& pdf, Vec3f& p, int base, Random& rnd, int samplingMode, int r, int tableWidth) {
    // YOUR CODE HERE (R2):
    // You should draw a random point on the light source and evaluate the PDF.
    // Store the results in "pdf" and "p".
    // 
    // Note: The "size" member is _one half_ the diagonal of the light source.
    // That is, when you map the square [-1,1]^2 through the scaling matrix
    // 
    // S = ( size.x    0   )
    //     (   0    size.y )
    // 
    // the result is the desired light source quad (see draw() function above).
    // This means the total area of the light source is 4*size.x*size.y.
    // This has implications for the computation of the PDF.

    // For extra credit, implement QMC sampling using some suitable sequence.
    // Use the "base" input for controlling the progression of the sequence from
    // the outside. If you only implement purely random sampling, "base" is not required.

    // (this does not do what it's supposed to!)
    //pdf = 1.0f;
    //p = Vec4f(m_xform.getCol(3)).getXYZ();
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
