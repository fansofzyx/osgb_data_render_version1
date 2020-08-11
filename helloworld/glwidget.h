#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTime>
#include <QVector>
#include <Camera.h>
#include <QOpenGLFramebufferObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include<vector>
#include"osgNode.h"
#include <QElapsedTimer>
#include<string>
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
{
    Q_OBJECT
public:
    GLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~GLWidget();
	vec3f getCameraPos();
	std::string readFile(std::string fileName);

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void initializeGL() override;
	void shaderCode();
	void loadShader(const char * shaderSource, unsigned int & shader, unsigned int type);
	void searchDir(std::string dir);
protected:
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	QPointF _mouseOld;
	bool _mouse_press = false;

private:
	xCamera _cam;
	mat4f w2s;
	QMatrix4x4 _w2c;
	QMatrix4x4 _c2s;
	QMatrix4x4 _mvp;
	int  _mvpHandler, _viewPosHandler, _lightPosHandler;
	osgScene scene;
	unsigned int _shaderProgram;
	int frameCount = 0;
	QElapsedTimer _fpsTimer;
	std::vector<QString> files;
public:
	static int screenHeight;
	static float fov;
};

#endif
