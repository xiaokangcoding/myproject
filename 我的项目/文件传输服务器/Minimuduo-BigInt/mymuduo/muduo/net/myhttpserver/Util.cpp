#include "Util.h"
namespace Util
{
	string errorContent()
	{
		string content;
		string tmp;
		string webHome="/root/muduo/03/jmuduo/muduo/net/myhttp/error.html";
		ifstream f(webHome);
		while(NULL!=getline(f,tmp))
		{
			content+=tmp;
		}
		f.close();
		return content;
	}
	//获取请求的绝对路径
	string ConstructPath(const string &path)
	{
		string webHome="/root/muduo/03/jmuduo/muduo/net/myhttp";
		if(path=="/")
		{
			return webHome+"/index.html";
		}
		else
		{
			return webHome+path;
		}
	}

	//获取文件扩展名
	string GetExtent(const string &path)
	{
		int i;
		for(i=path.size()-1;i>=0;--i)
		{
			if(path[i]=='.')
				break;
		}
		if(i==-1)
			return "";

		return string(path.begin()+i+1,path.end());
	}
	//获取content-Type
	void GetContentType(const string &tmpExtension,
			string &contentType)
	{
		string filepath="/root/muduo/03/jmuduo/muduo/net/myhttp/mime.types";
		ifstream mimeFile(filepath);
		string line;
		if(mimeFile.fail())
			cout<<"open error"<<endl;
		while(NULL!=getline(mimeFile,line))
		{
			if(line[0]!='#')
			{
				stringstream lineStream(line);
				contentType.clear();
				lineStream>>contentType;
				vector<string>extensions;
				string extension;
				while(lineStream>>extension)
				{
					extensions.push_back(extension);
				}
				for(int i=0;i<extensions.size();++i)
				{
					if(tmpExtension==extensions[i])
					{
						mimeFile.close();
						return ;
					}
				}

			}
		}
		//如果都不匹配就默认为text/plain
		contentType="text/plain";
		mimeFile.close();
	}
	//获取Content
	string GetContent(const string &fileName)
	{
		std::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
		if(fin.fail())
		{
			return string("");
		}
		std::ostringstream oss;
		oss << fin.rdbuf();
		return std::string(oss.str());
	}
}
