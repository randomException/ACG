#include "Radiosity.hpp"
#include "AreaLight.hpp"
#include "RayTracer.hpp"



namespace FW {


    // --------------------------------------------------------------------------

    Radiosity::~Radiosity()
    {
        if (isRunning())
        {
            m_context.m_bForceExit = true;
            while (m_launcher.getNumTasks() > m_launcher.getNumFinished())
                Sleep(1);
            m_launcher.popAll();
        }
    }

    Vec2f toUnitDisk(Vec2f onSquare) {
        if (onSquare.x == 0 && onSquare.y == 0) {
            return Vec2f(0, 0);
        }
        
        float phi, r;
        float a = onSquare.x;
        float b = onSquare.y;
        if (a > -b) {
            if (a > b) {
                r = a;
                phi = (FW_PI / 4) * (b / a);
            }
            else {
                r = b;
                phi = (FW_PI / 4) * (2 - (a / b));
            }
        }
        else {
            if (a < b) {
                r = -a;
                phi = (FW_PI / 4) * (4 + (b / a));
            }
            else {
                r = -b;
                if (b != 0) {
                    phi = (FW_PI / 4) * (6 - (a / b));
                }
                else {
                    phi = 0;
                }
            }

        }

        float u = r * cos(phi);
        float v = r * sin(phi);
        return Vec2f(u, v);
    }


    // --------------------------------------------------------------------------
    void Radiosity::vertexTaskFunc(MulticoreLauncher::Task& task)
    {
        RadiosityContext& ctx = *(RadiosityContext*)task.data;

        if (ctx.m_bForceExit)
            return;

        // which vertex are we to compute?
        int v = task.idx;

        // fetch vertex and its normal
        Vec3f n = ctx.m_scene->vertex(v).n.normalized();
        Vec3f o = ctx.m_scene->vertex(v).p + 0.01f*n;

        // YOUR CODE HERE (R3):
        // This starter code merely puts the color-coded normal into the result.
        //
        // In the first bounce, your task is to compute the direct irradiance
        // falling on this vertex from the area light source.
        // In the subsequent passes, you should compute the irradiance by a
        // hemispherical gathering integral. The commented code below gives you
        // an idea of the loop structure. Note that you also have to account
        // for how diffuse textures modulate the irradiance.

        //ctx.m_vecResult[ v ] = n*0.5+0.5;
        //Sleep(1);
        //return;

        ///*
        // direct lighting pass? => integrate direct illumination by shooting shadow rays to light source
        Random rnd;
        if (ctx.m_currentBounce == 0)
        {
            Vec3f E(0);
            int tableWidth = std::sqrt(ctx.m_numDirectRays);

            for (int r = 0; r < ctx.m_numDirectRays; ++r)
            {
                if (r > std::pow(tableWidth, 2) && ctx.m_SamplingMode == 2) {
                    break;
                }
                // draw sample on light source
                float pdf;
                Vec3f Pl;
                Vec3f p;

                ctx.m_light->sample(pdf, p, 0, rnd, ctx.m_SamplingMode, r, tableWidth);

                // construct vector from current vertex (o) to light sample
                Pl = p - o;

                RaycastResult result = ctx.m_rt->raycast(o, Pl * 0.99f);
                // trace shadow ray to see if it's blocked
                if (result.tri == nullptr && FW::dot(n, normalize(Pl)) > 0 && FW::dot(normalize(ctx.m_light->getNormal()), normalize(-Pl)) > 0)
                {
                    // if not, add the appropriate emission, 1/r^2 and clamped cosine terms, accounting for the PDF as well.
                    // accumulate into E
                    float l = Pl.length();
                    float cos_li = FW::dot(n, normalize(Pl));
                    float cos_i = FW::dot(normalize(ctx.m_light->getNormal()), normalize(-Pl));

                    E += ctx.m_light->getEmission() * cos_li * cos_i / (pow(l, 2) * pdf);
                }
            }
            // Note we are NOT multiplying by PI here;
            // it's implicit in the hemisphere-to-light source area change of variables.
            // The result we are computing is _irradiance_, not radiosity.
            if (ctx.m_SamplingMode != 2)
            {
                ctx.m_vecCurr[v] = E * (1.0f / ctx.m_numDirectRays);
            }
            else
            {
                ctx.m_vecCurr[v] = E * (1.0f / std::pow(tableWidth, 2));
            }
            //ctx.m_vecCurr[ v ] = E * (1.0f/ctx.m_numDirectRays);
            ctx.m_vecResult[v] = ctx.m_vecCurr[v];

            ctx.iterationResults[1][v] = ctx.m_vecCurr[v];
        }
        else
        {
            // OK, time for indirect!
            // Implement hemispherical gathering integral for bounces > 1.

            // Get local coordinate system the rays are shot from.
            Mat3f B = formBasis(n);

            Vec3f E(0.0f);
            int tableWidth = std::sqrt(ctx.m_numHemisphereRays);
            for (int r = 0; r < ctx.m_numHemisphereRays; ++r)
            {
                // Draw a cosine weighted direction and find out where it hits (if anywhere)
                // You need to transform it from the local frame to the vertex' hemisphere using B.

                if (r > std::pow(tableWidth, 2) && ctx.m_SamplingMode == 2) {
                    break;
                }

                Vec2f vec2D;

                // UNIFORM SAMPLING
                if (ctx.m_SamplingMode == 0) {
                    vec2D = Vec2f(1, 1);
                    while (pow(vec2D.x, 2) + pow(vec2D.y, 2) > 1) {
                        vec2D = rnd.getVec2f(-1, 1);
                    }
                }

                // STRATIFIED SAMPLING
                else if (ctx.m_SamplingMode == 2) {
                    int x = r % tableWidth;
                    int y = r / tableWidth;

                    //Bottom-left corner of unit square
                    Vec2f pointInSquare = Vec2f(-1, -1);
                    //Bottom-left corner of sub area
                    pointInSquare.x += (2.0f / tableWidth) * x;
                    pointInSquare.y += (2.0f / tableWidth) * y;
                    // Center of sub area
                    pointInSquare.x += (2.0f / tableWidth) * 0.5f;
                    pointInSquare.y += (2.0f / tableWidth) * 0.5f;
                    //Random point in sub area
                    Vec2f multiplier = rnd.getVec2f(-1, 1);
                    pointInSquare.x += (2.0f / tableWidth) * 0.5f * multiplier.x;
                    pointInSquare.y += (2.0f / tableWidth) * 0.5f * multiplier.y;

                    vec2D = toUnitDisk(pointInSquare);
                }

                // HALTON SAMPLING
                // ctx.m_SamplingMode == 1
                else {
                    // We can use the halton function from arealight
                    float multiplier_x = ctx.m_light->halton(r, 2);
                    float multiplier_y = ctx.m_light->halton(r, 3);

                    Vec2f pointInSquare = Vec2f(-1, -1) + Vec2f(2 * multiplier_x, 2 * multiplier_y);
                    vec2D = toUnitDisk(pointInSquare);
                }

                float z = FW::sqrt(1 - pow(vec2D.x, 2) - pow(vec2D.y, 2));
                Vec3f vec3D = Vec3f(vec2D.x, vec2D.y, z);

                // Make the direction long but not too long to avoid numerical instability in the ray tracer.
                // For our scenes, 100 is a good length. (I know, this special casing sucks.)
                Vec3f d = B * vec3D * 100;

                // Shoot ray, see where we hit
                const RaycastResult result = ctx.m_rt->raycast(o, d);
                if (result.tri != nullptr)
                {
                    // interpolate lighting from previous pass
                    const Vec3i& indices = result.tri->m_data.vertex_indices;

                    // check for backfaces => don't accumulate if we hit a surface from below!
                    if (FW::dot(d, result.tri->normal()) > 0) {
                        continue;
                    }

                    // fetch barycentric coordinates
                    // E(y) = a*E + b*E + (1-a-b)*E
                    float a = result.u;
                    float b = result.v;

                    // Ei = interpolated irradiance determined by ctx.m_vecPrevBounce from vertices using the barycentric coordinates
                    Vec3f Ei = (1 - a - b) * ctx.m_vecPrevBounce[indices[0]]
                        + a * ctx.m_vecPrevBounce[indices[1]]
                        + b * ctx.m_vecPrevBounce[indices[2]];

                    // Divide incident irradiance by PI so that we can turn it into outgoing
                    // radiosity by multiplying by the reflectance factor below.
                    Ei *= (1.0f / FW_PI);

                    // check for texture
                    const auto mat = result.tri->m_material;
                    if (mat->textures[MeshBase::TextureType_Diffuse].exists())
                    {
                        Vec2f uv = (1 - result.u - result.v) * result.tri->m_vertices[0].t
                            + result.u * result.tri->m_vertices[1].t
                            + result.v * result.tri->m_vertices[2].t;
                        // read diffuse texture like in assignment1

                        const Texture& tex = mat->textures[MeshBase::TextureType_Diffuse];
                        const Image& teximg = *tex.getImage();

                        Vec2i texelCoords = getTexelCoords(uv, teximg.getSize());
                        Ei *= teximg.getVec4f(texelCoords).getXYZ();

                    }
                    else
                    {
                        // no texture, use constant albedo from material structure.
                        // (this is just one line)
                        Ei *= mat->diffuse.getXYZ();
                    }

                    E += Ei;	// accumulate
                }
            }
            // Store result for this bounce
            // Note that since we are storing irradiance, we multiply by PI(
            // (Remember the slides about cosine weighted importance sampling!)
            if (ctx.m_SamplingMode != 2)
            {
                ctx.m_vecCurr[v] = E * (FW_PI / ctx.m_numHemisphereRays);
            }
            else
            {
                ctx.m_vecCurr[v] = E * (FW_PI / std::pow(tableWidth, 2));
            }
            // Also add to the global accumulator.
            ctx.m_vecResult[v] = ctx.m_vecResult[v] + ctx.m_vecCurr[v];

            // uncomment this to visualize only the current bounce
            //ctx.m_vecResult[ v ] = ctx.m_vecCurr[ v ];
            ctx.iterationResults[0][v] = ctx.m_vecResult[v];
            ctx.iterationResults[ctx.m_currentBounce + 1][v] = ctx.m_vecCurr[v];
        }

        return;
    }
    // --------------------------------------------------------------------------

    void Radiosity::startRadiosityProcess(MeshWithColors* scene, AreaLight* light, RayTracer* rt, int numBounces, int numDirectRays, int numHemisphereRays, int samplingMode)
    {
        // put stuff the asyncronous processor needs 
        m_context.m_scene = scene;
        m_context.m_rt = rt;
        m_context.m_light = light;
        m_context.m_currentBounce = 0;
        m_context.m_numBounces = numBounces;
        m_context.m_numDirectRays = numDirectRays;
        m_context.m_numHemisphereRays = numHemisphereRays;

        m_context.m_SamplingMode = samplingMode;
        // Precalculate halton sample points
        /*if (m_context.m_SamplingMode == 1) {
            int tableWidth = std::sqrt(m_context.m_numHemisphereRays);
            for (int r = 0; r < tableWidth; ++r)
            {
                m_context.haltonBase2.push_back(light->halton(r, 2));
                m_context.haltonBase2.push_back(light->halton(r, 3));
            }
        }*/

        // resize all the buffers according to how many vertices we have in the scene
        m_context.m_vecResult.resize(scene->numVertices());
        m_context.m_vecCurr.resize(scene->numVertices());
        m_context.m_vecPrevBounce.resize(scene->numVertices());
        m_context.m_vecResult.assign(scene->numVertices(), Vec3f(0, 0, 0));

        //EXTRA
        m_context.iterationResults.resize(10);
        for (int i = 0; i < 10; i++) {
            m_context.iterationResults[i].resize(scene->numVertices());
        }

        m_context.m_vecSphericalC.resize(scene->numVertices());
        m_context.m_vecSphericalX.resize(scene->numVertices());
        m_context.m_vecSphericalY.resize(scene->numVertices());
        m_context.m_vecSphericalZ.resize(scene->numVertices());

        m_context.m_vecSphericalC.assign(scene->numVertices(), Vec3f(0, 0, 0));
        m_context.m_vecSphericalX.assign(scene->numVertices(), Vec3f(0, 0, 0));
        m_context.m_vecSphericalY.assign(scene->numVertices(), Vec3f(0, 0, 0));
        m_context.m_vecSphericalZ.assign(scene->numVertices(), Vec3f(0, 0, 0));

        // fire away!
        m_launcher.setNumThreads(m_launcher.getNumCores());	// the solution exe is multithreaded
                                                            //m_launcher.setNumThreads(1);							// but you have to make sure your code is thread safe before enabling this!
        m_launcher.popAll();
        m_launcher.push(vertexTaskFunc, &m_context, 0, scene->numVertices());
    }
    // --------------------------------------------------------------------------

    void Radiosity::updateMeshColors(std::vector<Vec4f>& spherical1, std::vector<Vec4f>& spherical2, std::vector<float>& spherical3, bool spherical)
    {
        // Print progress.
        printf("%.2f%% done     \r", 100.0f*m_launcher.getNumFinished() / m_context.m_scene->numVertices());

        // Copy irradiance over to the display mesh.
        // Because we want outgoing radiosity in the end, we divide by PI here
        // and let the shader multiply the final diffuse reflectance in. See App::setupShaders() for details.
        for (int i = 0; i < m_context.m_scene->numVertices(); ++i) {

            // Packing data for the spherical harmonic extra.
            // In order to manage with fewer vertex attributes in the shader, the third component is stored as the w components of other actually three-dimensional vectors.
            if (spherical) {
                auto mult = (2.0f / FW_PI);
                m_context.m_scene->mutableVertex(i).c = m_context.m_vecSphericalC[i] * mult;
                spherical3[i] = m_context.m_vecSphericalZ[i].x *mult;
                spherical1[i] = Vec4f(m_context.m_vecSphericalX[i], m_context.m_vecSphericalZ[i].y) *mult;
                spherical2[i] = Vec4f(m_context.m_vecSphericalY[i], m_context.m_vecSphericalZ[i].z) *mult;
            }
            else {
                m_context.m_scene->mutableVertex(i).c = m_context.m_vecResult[i] * (1.0f / FW_PI);
            }
        }
    }
    // --------------------------------------------------------------------------
    void Radiosity::updateMeshColorsWithSpecificBounce(MeshWithColors* scene, int bounce, bool m_bouncesExists) {
        m_context.m_scene = scene;

        // If "show certain bounce" is pressed before actual radiosity calculations, initialize all colors to black
        if (!m_bouncesExists) {
            m_context.iterationResults.resize(10);
            for (int i = 0; i < 10; i++) {
                m_context.iterationResults[i].resize(scene->numVertices());
            }
        }

        int vertices = m_context.m_scene->numVertices();
        for (int i = 0; i < vertices; i++) {
            m_context.m_scene->mutableVertex(i).c = m_context.iterationResults[bounce + 1][i] * (1.0f / FW_PI);
        }
    }

    void Radiosity::checkFinish()
    {
        // have all the vertices from current bounce finished computing?
        if (m_launcher.getNumTasks() == m_launcher.getNumFinished())
        {
            // yes, remove from task list
            m_launcher.popAll();

            // more bounces desired?
            if (m_context.m_currentBounce < m_context.m_numBounces)
            {
                // move current bounce to prev
                m_context.m_vecPrevBounce = m_context.m_vecCurr;
                ++m_context.m_currentBounce;
                // start new tasks for all vertices
                m_launcher.push(vertexTaskFunc, &m_context, 0, m_context.m_scene->numVertices());
                printf("\nStarting bounce %d\n", m_context.m_currentBounce);
            }
            else printf("\n DONE!\n");
        }
    }
    // --------------------------------------------------------------------------

} // namespace FW
