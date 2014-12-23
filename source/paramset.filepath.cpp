#include "paramset.filepath.h"
using namespace imajuscule;

FilePath::FilePath() :
ParamSet("FilePath", paramsInSet{ Param<std::string>("File Path", std::string()) })
{
}

FilePath::~FilePath()
{
}

void FilePath::getPath(std::string & path)
{
    static_cast<Param<std::string>&> (getParam(0)).GetValue(path);
}
