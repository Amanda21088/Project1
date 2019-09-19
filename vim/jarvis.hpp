#ifndef _JARVIS_
#define _JARVIS_
#include<unistd.h>
#include<cstdio>
#include"base/http.h"
#include<sstream>
#include <iostream> 
#include <cstdio>
#include <fstream> 
#include <string>
#include <map>
#include <unistd.h> 
#include <sys/types.h>
#include"speech.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unordered_map>
#include<json/json.h>
#include<memory>
#include<pthread.h>

#define SPEECH_FILE "temp_file/demo.wav"
#define PLAY_FILE "temp_file/play.mp3"
#define CMD_ETC "command.etc"

class InterRobot
{
  private:
    std::string url = "openapi.tuling123.com/openapi/api/v2";
    std::string api_key = "e541c4d277d74e04a166e2fdaa16597e";
    std::string user_id = "1";
    aip::HttpClient client;
  public:
    InterRobot()
    {}
    Json::Value PostRequest(Json::Value data)
    {
      std::string response;
      Json::Value obj;
      Json::CharReaderBuilder crbuilder;
      int code = this->client.post(url, nullptr, data, nullptr, &response);
      if(code != CURLcode::CURLE_OK){
        obj[aip::CURL_ERROR_CODE] = code;
        return obj;
      }
      std::string error;
      std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
      reader->parse(response.data(), response.data()+response.size(), &obj, &error);
      return obj;
    }
    std::string Talk(std::string &message)
    {
        Json::Value root;
        Json::Value item1;
        Json::Value item2;
        root["reqType"] = 0;
        item1["text"] = message;
        item2["inputText"] = item1;
        item1.clear();
        root["perception"] = item2;
        item2.clear();
        item2["apiKey"] = api_key;
        item2["userId"] = user_id;
        root["userInfo"] = item2;
        item2.clear();

        Json::Value ret = PostRequest(root);
        Json::Value _result = ret["results"];
        Json::Value values = _result[0]["values"];
        std::cout << "Robot: " << values["text"].asString() << std::endl;
        return values["text"].asString();
    }
        ~InterRobot()
        {}


  public:
  void xulie()
  {
      Json::Value root;
      Json::StreamWriterBuilder wb;
      std::ostringstream os;

      root["name"] = "zhangsan";
      root["age"] = 26;
      root["high"] = "186.5i";
    
      std::unique_ptr<Json::StreamWriter> jsonWriter(wb.newStreamWriter());
      jsonWriter->write(root, &os);
      std::string s = os.str();
      std::cout << "[Me]: "<<  s << std::endl;

  }
  void ParseJson(std::string &s)
  {
    JSONCPP_STRING errs;                                                             Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
    bool res = jsonReader->parse(s.data(),s.data()+s.size(), &root, &errs);
    if(!res || !errs.empty())
    {
      std::cerr << "json parse error: " << errs <<std::endl;
      return;
    }
 
      std::cout << "Name" << root["Name"].asString() << std::endl;
      std::cout << "Age" << root["Age"].asInt() << std::endl;
      std::cout << "High" << root["High"].asFloat() << std::endl;
   }
};


class SpeechRec{
  private:
    std::string app_id = "16873530";
    std::string api_key = "Moou7QNyYFymGPHupxhxB9wu";
    std::string secret_key = "6abHnK5Ow0nVuUoluq70447Z1AvpRB53";
    aip::Speech *client;
  public:
    SpeechRec()
    {
      client = new aip::Speech(app_id, api_key, secret_key);
    }
    void ASR(int &err_code, std::string &message)
    {
      std::cout << std::endl << "正在识别。。。" << std::endl;
      std::map<std::string, std::string> options;//以中文显示i
      options["dev_pid"] = "1536";
      //options["lan"] = "ZH";

      std::string file_content;
      aip::get_file_content(SPEECH_FILE, &file_content);//获取你的语音文件
      //开始识别，需要你的语音文件，格式"wav",频率是16K
      Json::Value result = client->recognize(file_content, "wav", 16000, options);
      err_code = result["err_no"].asInt();
      if(err_code == 0)
      {
        message = result["result"][0].asString();
      }
      else{
        message = "识别错误。。。";
      }
  
    }
    //语音合成Text to Speech
    void TTS(std::string message)
    {
      std::ofstream ofile;
      std::string file_ret;
      std::map<std::string, std::string> options;
      options["spd"] = "5";
      options["per"] = "3";
      options["pit"] = "6";

      ofile.open(PLAY_FILE, std::ios::out | std::ios::binary);
      //语音合成，将文本转成语音，放到指定目录，形成指定文件
      Json::Value result = client->text2audio(message, options, file_ret);
      if(!file_ret.empty())
      {
        ofile << file_ret;
      }
      else{
        std::cout << "error" << result.toStyledString();
      }
      ofile.close();
    }
    ~SpeechRec()
    {
      delete client;
      client = NULL;
    }
};
class jarvis{
    private:
      SpeechRec sr;
      InterRobot robot;
      static pthread_t id;
      std::unordered_map<std::string, std::string> command_set;
    public:
      //加载命令执行配置文件
      jarvis()
      {
        char buffer[256];
        std::ifstream in(CMD_ETC);
        if(!in.is_open())
        {
          std::cerr << "open file error" << std::endl;
          exit(1);
        }
        std::string sep = ":";
        while(in.getline(buffer, sizeof(buffer)))
        {
          std::string str = buffer;
          std::size_t pos = str.find(sep);
          if(std::string::npos == pos)
          {
            std::cerr << "Load Etc Error" << std::endl;
            exit(2);
          }
          std::string k = str.substr(0, pos);
          std::string v = str.substr(pos+sep.size());
          k += "。";
          command_set.insert(std::make_pair(k, v));
        }
        std::cout << "Load command etc...done" << std::endl;
        in.close();
      }
      //在Linux中执行指定命令，采用popen。介绍一下popen
      bool Exec(std::string command, bool print)
      {
        //if(!print)
         // command += " >/dev/null 2>&1";
        //std::cout << command << std::endl;
        FILE* fp = popen(command.c_str(), "r");
        if(NULL == fp)
        {
          std::cerr << "popen error!" << std::endl;
          return false;
        }
        if(print)
        {
          char c;
          std::cout << "----------start-------------" << std::endl;
          while(fread(&c, 1, 1, fp) > 0)
          {
            std::cout << c;
          }
          std::cout << std::endl;
          std::cout << "-----------end--------------" << std::endl;
        }
        pclose(fp);
        return true;
      }
      static void *ThreadRun(void*arg)
      {
        const char *tips = (char*)arg;
        int i = 0;
        char bar[103] = {0};
        const char *lable = "|/-\\";
        for(; i <= 50; i++)
        {
          printf("%s[%-51s][%d%%][%c]\r", tips, bar, i*2, lable[i%4]);
          fflush(stdout);
          bar[i] = '=';
          bar[i+1] = '>';

          usleep(49000*2);
        }
        printf("\n");
      }
      static void PrintfStart(std::string tips)
      {
        pthread_create(&id, NULL, ThreadRun, (void*)tips.c_str());
      }
      static void PrintEnd()
      {
        pthread_cancel(id);
      }
      //判断消息是否是需要执行的命令，如果是命令，需要执行他，而不需要交给图灵机器人对话
      bool MessageIsCommand(std::string _message, std::string &cmd)
      {
        std::unordered_map<std::string, std::string>::iterator iter = command_set.find(_message);
        if(iter != command_set.end())
        {
          cmd = iter->second;
          return true;
        }
        cmd = "";
        return false;
      }
      //使用arecord工具进行录音，并进行语音识别，语音转文本
      bool RecordAndASR(std::string& message)
      {
        int err_code = -1;
        std::string record = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
        record += SPEECH_FILE;
        record += ">/dev/null 2>&1"; //不显示输出结果或者消息

        std::string tips = "Record ... ";
        jarvis::PrintfStart(tips);
        std::cout << "请讲话...";
        fflush(stdout);
        bool ret = Exec(record, false);
        jarvis::PrintEnd();
        if(ret)
        {
          sr.ASR(err_code, message);
          if(err_code == 0)
        	{
            return true;
	        }	
       	    std::cout << "语音识别失败..." << std::endl;
        }
        else{
            std::cout << "录制失败..." << std::endl;
         }
        return false;
      }
      //语音合成
      bool TTSAndPlay(std::string message)
      {
        //cvlc命令行式的播放
        std::string play = "cvlc --play-and-exit ";
        play += PLAY_FILE;
        play += " >/dev/null 2>&1"; 
        sr.TTS(message);
        Exec(play, false);
        return true;
      }
      void Run()
      {

        //PrintfStart("record:");
        //PrintEnd();
        //Exec("rm ./temp_file/demo.wav", false);
        volatile bool quit = false;
        std::string message;
        while(!quit)
        {   
          message = "";
          bool ret = RecordAndASR(message);
          if(ret)
          {
            std::string cmd;
            std::cout << "我：" << message << std::endl;
            if(MessageIsCommand(message, cmd))
            {
              if(message == "退出。")
              {
                TTSAndPlay("好的");
                std::cout << "我走了，不要想我哦" << std::endl;
                quit = true;
              }
              else{
                Exec(cmd, true);
              }
            }
            else{
              //不是命令，就交给图灵机器人识别
              std::string play_message = robot.Talk(message);
              TTSAndPlay(play_message);
            }
          }
        }
      }

      ~jarvis()
      {}
};
pthread_t jarvis::id=0;
#endif

