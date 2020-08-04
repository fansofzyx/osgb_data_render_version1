#include "osgNode.h"
#include <QMutexLocker>
#include"Camera.h"
#include"Drawable.h"
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QFile>
#include<iostream>
#include"glwidget.h"
#include<QProcess>
#include<QLockFile>
#include<cstdio>
int osgScene::DrawableLoadOnFrame = 50;
vec3f osgNode::_nowCamPos;
xCamera osgNode::_nowCam;
QString osgb2JsonThread::exeDir = "C:/Users/zheng_group/Desktop/osgb/osgbdebuggerCPLPL.exe";
osgNode::osgNode()
{
	 _nextFile = nullptr;
	 _thisFile = nullptr;
}

osgNode::~osgNode()
{
	
}
//计算像素尺寸
float osgNode::calculateDistance()
{
	
	float xD = (_nowCamPos.x - _bs[0]);
	float yD = (_nowCamPos.y - _bs[1]);
	float zD = (_nowCamPos.z - _bs[2]);
	xD *= xD;
	yD *= yD;
	zD *= zD;
	return sqrt(xD + yD + zD);
}
double osgNode::calculatePixel()
{
	float distance = calculateDistance();
	float radius = _bs[3];
	double angularSize = (2.0*std::atan(radius / distance)) / 3.14159 * 180;
	double dpp = std::max((double)60, 1.0e-17) / (1200);
	return angularSize / dpp;
}
//获取drawables的状态，loaded 的情况是ready
bool osgNode::getDABstates()
{
	
	for (int i = 0; i < _drawables.size(); i++)
	{
		if (_drawables[i]->isReady() == false)
			return false;
	}
	return true;
}
//获取下一个文件的地址
QString osgNode::getNextFileName()
{
	return _dir + "/" + _nextList[1];
}
void osgNode::draw()
{	float pixel = 0;
	bool isFlag = false;
	//视域范围之外的剔除
	if (!_nowCam.sphereInFrustum(vec3f(_bs[0], _bs[1], _bs[2]), _bs[3]))
		return;
	
	if (_nextFile != nullptr)
		pixel = calculatePixel();

	if ((_nextFile==nullptr||pixel <= _nextValue[1]))
	{
		DABdraws();
	}
	else
	{
		if (_nextFile->getFilestates())
		{
			_nextFile->render();
		}
		else
		{
			if (this->getDABstates())
			DABdraws();
			_nextFile->_fakeRender();
		}
	}
}
void osgNode::fakeDraw()
{
	for (int i = 0; i < _drawables.size(); i++)
	{
		if (!_drawables[i]->isReady())
			_drawables[i]->draw();
	}
}
void osgNode::DABdraws()
{
	for (int i = 0; i < _drawables.size(); i++)
	{
		(_drawables[i])->draw();
	}
}
void osgNode::set_bs(vec4f bs)
{
	_bs = bs;
}

void osgNode::set_dir(QString dir)
{
	_dir = dir;
}
void osgNode::set_nextLsit(QString list1,QString list2)
{
	_nextList[0] = list1;
	_nextList[1] = list2;
}
void osgNode::set_nextValue(vec4f nextValue)
{
	_nextValue = nextValue;
}
void osgNode::add_drawable(Drawable * dab)
{
	_drawables.push_back(dab);
}
void osgNode::set_camPos(vec3f camPos,xCamera cam)
{
	_nowCamPos = camPos;
	_nowCam = cam;
}
void osgNode::set_nextFile(osgFile* nextFile)
{
	_nextFile = nextFile;
}
void osgNode::set_thisFile(osgFile* thisFile)
{
	_thisFile = thisFile;
}



osgFile::osgFile(const QString& filePath, osgb2JsonThread* fileLoadThread)
{
	_path = filePath;
	_state = FILE_NEW;
	_fileLoadThread = fileLoadThread;
}

osgFile::~osgFile()
{

}
bool osgFile::getFileReady()
{
	if (_state == FILE_LOADED)
		return true;
	return false;
}
void osgFile::render()
{
	
	switch (_state)
	{
		case FILE_NEW:
			onNew();
			break;
		case FILE_LOADING:
			onLoading();
			break;
		case FILE_LOADED:
			onLoaded();
			break;
	}
}
void osgFile::_fakeRender()
{
	switch (_state)
	{
	case FILE_NEW:
		onNew();
		break;
	case FILE_LOADING:
		onLoading();
		break;
	case FILE_LOADED:
		onFakeLoaded();
		break;
	}
}
void osgFile::onFakeLoaded()
{
	for (int i = 0; i <_nodes.size(); i++)
	{
		_nodes[i]->fakeDraw();
	}
}
bool osgFile::getFilestates()
{
	if (_state != FILE_LOADED)
		return false;
	for (int i = 0; i < _nodes.size(); i++)
	{
		if (_nodes[i]->getDABstates() == false)
			return false;
	}
	return true;
}
void osgFile::onNew()
{
	if (_fileLoadThread->append(this, _path))
	{
		_state = FILE_LOADING;
	}
}
void osgFile::onLoading()
{
	if (_fileLoadThread->query(this, _nodes))
	{
		_state = FILE_LOADED;
	}
}

bool osgFile::onLoaded()
{
	bool isFlag = true;
	//qWarning() << _nodes.size();
	for (int i = 0; i < _nodes.size(); i++)
	{
		_nodes[i]->draw();
	}
	return isFlag;
}
osgb2JsonThread::osgb2JsonThread()
{

}

osgb2JsonThread::~osgb2JsonThread()
{

}

void osgb2JsonThread::run()
{
	while (!isInterruptionRequested()) {
		OsgbFilePath curPath(nullptr, "");
		{
			QMutexLocker locker(&_locker);
			if (_paths.size() > 0)
			{
				curPath = _paths.front();
				//_paths.pop_front();
			}
			
		}
		if (curPath.first != nullptr)
		{
			std::vector<osgNode *> nodes;
			QFile file(curPath.second);
			if (file.open(QIODevice::ReadOnly))
			{
				QByteArray data = file.readAll();
				file.close();
				QJsonDocument doc = QJsonDocument::fromJson(data);
				if (doc.isArray())
				{
					QJsonArray osgArray = doc.array();
					for (int i = 0; i < osgArray.size(); i++)
					{
						osgNode * tempOsgNode = new osgNode();
						QJsonObject temp = osgArray[i].toObject();
						QString dir;
						//获取数据并且存入osgNode;
						if (temp.find("DataBasePath") != temp.end())
						{
							dir = temp.value("DataBasePath").toString();
							tempOsgNode->set_dir(dir);
						}
						if (temp.find("CenterValue") != temp.end())
						{
							QJsonArray jsonArray = temp.value("CenterValue").toArray();
							vec4f bs;
							for (int i = 0; i < jsonArray.size(); i++)
							{
								bs[i] = (float)jsonArray[i].toDouble();
							}
							tempOsgNode->set_bs(bs);
						}
						if (temp.find("Drawable") != temp.end())
						{
							QJsonArray jsonArray = temp.value("Drawable").toArray();
							for (int i = 0; i < jsonArray.size(); i++)
							{
								QJsonObject dabJson = jsonArray[i].toObject();
								QString da = dir + dabJson.value("DataFile").toString();
								QString te = dir + dabJson.value("TextureName").toString();
								Drawable * tempDraw = new Drawable(da, te, _loaderDAB,curPath.second);
								tempOsgNode->add_drawable(tempDraw);
							}
						}
						if (temp.find("RangeList") != temp.end())
						{
							QJsonArray jsonArray = temp.value("RangeList").toArray();
							vec4f rlist;
							for (int i = 0; i < jsonArray.size(); i++)
							{
								rlist[i] = (float)jsonArray[i].toDouble();
							}
							tempOsgNode->set_nextValue(rlist);
						}
						if (temp.find("NextRangeFile") != temp.end())
						{
							QJsonArray jsonArray = temp.value("NextRangeFile").toArray();
							if (jsonArray.size() == 1)
							{
								tempOsgNode->set_nextLsit(dir + "", dir + jsonArray[0].toString() + ".json");
								tempOsgNode->set_nextFile(new osgFile(dir + jsonArray[0].toString() + ".json", this));
							}

						}
						else tempOsgNode->set_nextFile(nullptr);
						tempOsgNode->set_thisFile(curPath.first);
						nodes.push_back(tempOsgNode);
						std::remove(curPath.second.toStdString().c_str());
					}
				}
				QMutexLocker locker(&_lockerQuery);
				_files[curPath.first] = nodes;
				//然后跳出
				{	
					QMutexLocker locker(&_locker);
					if (_paths.size() > 0)
					{
						//curPath = _paths.front();
						_paths.pop_front();
					}
				}
			}
			else
			{
				/*
				QStringList tempStrList;
				QString jsonDir = curPath.second;
				QString dir = curPath.second;
				int ind = jsonDir.lastIndexOf(".");
				jsonDir = jsonDir.left(ind+1);
				jsonDir += "osgb";
				tempStrList.append(jsonDir);
				ind = dir.lastIndexOf("/");
				dir = dir.left(ind);
				tempStrList.append(dir);
				qWarning() << "FILES" << jsonDir;
				
				QLockFile fLocker(curPath.second);
				fLocker.lock();
				QProcess p;
				p.start(exeDir, tempStrList);
				p.waitForStarted();
				p.waitForFinished();
				fLocker.unlock();
				*/
			}
		}
		else
		{
			msleep(10);
		}
	}
}


bool osgb2JsonThread::append(osgFile* fptr, const QString& filepath)
{
	if(_locker.try_lock())
	{
		_paths.push_back(OsgbFilePath(fptr, filepath));
		_locker.unlock();
		//test();
		return true;
	}
	return false;
}
bool osgb2JsonThread::query(osgFile* fptr, std::vector<osgNode*>& nodes)
{
	bool isFlag = false;
	if (_lockerQuery.try_lock())
	{
		if(_files.find(fptr)!=_files.end())
		{
			nodes = _files[fptr];
			isFlag = true;
		}
		_lockerQuery.unlock();
	}
	
	return isFlag;
}
void osgb2JsonThread::test()
{
	
	
}
void osgb2JsonThread::setThread(drawDataThread* loaderDAB)
{
	_loaderDAB = loaderDAB;
}

osgScene::osgScene()
{
	_loader = new osgb2JsonThread();
	_loaderDAB = new drawDataThread();
	_loader->setThread(_loaderDAB);
	_loader->start();
	_loaderDAB->start();
}

osgScene::~osgScene()
{

}

void osgScene::load(const QString& dataDir)
{
	osgFile * osgf = new osgFile(dataDir, _loader);
	_files.push_back(osgf);
}
void osgScene::render()
{
	DrawableLoadOnFrame = 50;
	for (int i = 0; i < _files.size(); i++)
	{
		
			_files[i]->render();
		
	}
}
