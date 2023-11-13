//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "LoadShaders.h"

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];

const GLuint  NumVertices = 6;

//----------------------------------------------------------------------------
//
// init
//

void
init( void )
{
    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Triangles] );

    GLfloat  vertices[NumVertices][2] = {
        { -0.90f, -0.90f }, {  0.85f, -0.90f }, { -0.90f,  0.85f },  // Triangle 1
        {  0.90f, -0.85f }, {  0.90f,  0.90f }, { -0.85f,  0.90f }   // Triangle 2
    };

    glCreateBuffers( NumBuffers, Buffers ); // 生成NumBuffers个缓冲，缓冲id保存到Buffers中
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer] ); // 使用第ArrayBuffer号buffer

    glBufferStorage( GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0); // 创建一个ARRAY_BUFFER 保存vertices数据 最后一个参数表示数据不会改变
    

    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders( shaders );
    glUseProgram( program );

    /**
     * 1. 着色器中变量位置，
     * 2. 设置着色器中vec3的前两个参数，
       3. GL_FLOAT是vec3的数据类型，
     * 4. GL_FALSE不归格化，0表示歩长，也可以用sizeof(float) * 2，
     这个参数的意思简单说就是从这个属性第二次出现的地方到整个数组0位置之间有多少字节。最后一个参数的类型是void*，所以需要我们进行这个奇怪的强制类型转换。它表示位置数据在缓冲中起始位置的偏移量(Offset)。
     * 5. 它表示位置数据在缓冲中起始位置的偏移量(Offset)。由于位置数据在数组的开头，所以这里是0。
     */
    glVertexAttribPointer( vPosition, 2, GL_FLOAT,
                           GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vPosition ); // 启用着色器中vPosition位置的顶点数组
}

//----------------------------------------------------------------------------
//
// display
//

void
display( void )
{
    static const float black[] = { 0.0f, 1.0f, 1.0f, 1.0f }; // 0-1的数字，1是白色，0是黑色
    // RGBA (透明度)
    glClearBufferfv(GL_COLOR, 0, black);

    glBindVertexArray( VAOs[Triangles] );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    // 测试其他种类
    // glDrawArrays( GL_TRIANGLE_FAN, 0, NumVertices );
}

//----------------------------------------------------------------------------
//
// main
//

#ifdef _WIN32
int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
)
#else
int
main( int argc, char** argv )
#endif
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangles", NULL, NULL);

    glfwMakeContextCurrent(window);
    gl3wInit();

    init();

    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}
