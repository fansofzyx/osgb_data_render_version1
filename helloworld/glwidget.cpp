
#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <vec3.h>
#include <mat4.h>
#include <Windows.h>
#include<io.h>
#include<string>
int GLWidget::screenHeight = 0;
float  GLWidget::fov = 0;
GLWidget::GLWidget(QWidget* parent /* = nullptr */, Qt::WindowFlags f /* = Qt::WindowFlags() */)
	:QOpenGLWidget(parent, f)
{
    setFixedSize(1600, 1200);
	scene = osgScene();
	searchDir("C:/Users/zheng_group/Desktop/Production_OSBG/Data/");
	for (int i = 0; i < files.size(); i++)
	{
		scene.load(files[i]);
	}
	
}

GLWidget::~GLWidget()
{
	
}
vec3f GLWidget::getCameraPos()
{
	return _cam.pos;
}
void GLWidget::searchDir(std::string dir)
{
	intptr_t handle;
	struct _finddata_t fileinfo;
	int done = 0;
	//��һ�β���
	handle = _findfirst((dir + "/*").c_str(), &fileinfo);
	if (handle == -1LL)
		return;
	do
	{
		if ((fileinfo.attrib &  _A_SUBDIR))
		{
			std::string str = fileinfo.name;
			if (str.size() > 5)
				files.push_back(QString::fromStdString((std::string(dir + "/" + fileinfo.name + "/" + fileinfo.name + ".json"))));
		}
		done = _findnext(handle, &fileinfo);
		//cout << ENOENT << endl;
	} while (done == 0 && handle != -1LL);

	_findclose(handle);
}
void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    _cam.near_dist = 0.1f;
	_cam.far_dist = 20000.0f;
	_cam.pos = vec3f(-235.98385620117188,
		-188.58071899414063,
		300.5219907760620117);
	_cam.view = vec3f(0.425733656, 0.279169768, -0.860706151).normalize();
	_cam.WalkSpeed = 10;
	shaderCode();
	osgNode::set_camPos(_cam.getPos(),_cam);
	_fpsTimer.start();
}
void GLWidget::shaderCode()
{
	const GLchar * vertSource = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 mvp;\n"
		"void main()\n"
		"{\n"
		//"gl_Position = vec4((aPos.x-102)*0.01,(aPos.y+71)*0.01,(aPos.z+14)*0.01, 1.0f);\n"
		"gl_Position = mvp*vec4(aPos, 1.0f);\n"
		"TexCoord = aTexCoord;\n"
		"}\n";
	const GLchar * fragSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture;\n"
		"void main()\n"
		"{\n"
		"FragColor = texture(ourTexture, vec2(TexCoord.x, 1-TexCoord.y));\n"
		"}\n";
	unsigned int vertShader, fragShader;
	loadShader(vertSource, vertShader, GL_VERTEX_SHADER);
	loadShader(fragSource, fragShader, GL_FRAGMENT_SHADER);

	//���ӳ���
	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertShader);
	glAttachShader(_shaderProgram, fragShader);
	glLinkProgram(_shaderProgram);

	//ɾ����ɫ��
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	_mvpHandler = glGetUniformLocation(_shaderProgram, "mvp");
	

}
void GLWidget::loadShader(const char * shaderSource, unsigned int & shader, unsigned int type)
{
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

}
void GLWidget::keyPressEvent(QKeyEvent *event)
{
	if (GetAsyncKeyState('W')) {
		_cam.GoFront();
	}
	if (GetAsyncKeyState('S')) {
		_cam.GoBack();
	}
	if (GetAsyncKeyState('A')) {
		_cam.GoLeft();
	}
	if (GetAsyncKeyState('D')) {
		_cam.GoRight();
	}
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_mouse_press = true;
	}
	_mouseOld = event->screenPos();
	
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (_mouse_press)
	{
		QPointF rot = (event->screenPos() - _mouseOld) * _cam.MouseSensitivity;
		_cam.RotateView(rot.x(), vec3f(0.0, 0.0, 1.0));

		vec3f rot_axis = vec3f(-_cam.view.y, _cam.view.x, 0.0);
		rot_axis = rot_axis.normalize();

		_cam.RotateView(-rot.y(), rot_axis);

		_mouseOld = event->screenPos();
	}
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_mouse_press = false;
	}
}

void GLWidget::paintGL()
{
	mat4f c2s = _cam.GetPerspectiveMat((float)width() / height());
	mat4f w2c = _cam.GetLookAtMat();
	w2s = c2s * w2c;
	_cam._mvp = w2s;
	_w2c = QMatrix4x4((const float*)w2c._m);
	_c2s = QMatrix4x4((const float*)c2s._m);
	_mvp = QMatrix4x4((const float*)w2s._m);
	glUseProgram(_shaderProgram);
	glUniformMatrix4fv(_mvpHandler, 1, GL_FALSE, _mvp.data());
    glClearColor(0.1f, 0.2, 0.2, 1.0f);
	glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_cam.updateFrustum();

	osgScene::DrawableLoadOnFrame = 1000;
	
	screenHeight = height();
	fov = _cam.fov;
	osgNode::set_camPos(_cam.getPos(),_cam);
//	qWarning() << "____" ;
	scene.render();
	QOpenGLWidget::update();
	static int fpscount = 0;
	frameCount++;
	fpscount++;
	if (fpscount == 100)
	{
		qWarning() << "fps: " << fpscount / (_fpsTimer.elapsed() * 0.001);
		_fpsTimer.restart();
		fpscount = 0;
	}
}
void GLWidget::resizeGL(int w, int h)
{
   
}
