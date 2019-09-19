#include<iostream>
#include<sstream>
#include<memory>
#include<string>
#include<json/json.h>
#include"jarvis.hpp"
int main()
{
  //InterRobot ir1;
  //ir1.xulie();

  InterRobot ir2;
  std::string s;
   // std::cout << "[Me]: ";
  //std::cin >> s;
 // std::string ret = ir2.Talk(s);
  //std::cout << ret << std::endl;
  jarvis js;
  js.Run();
  return 0;
}
