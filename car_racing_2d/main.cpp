#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

namespace {
constexpr float W = 480.0f, H = 720.0f;
struct Car { float x, y, speed; int lane; };
GLFWwindow* window = nullptr;
float playerX = 240, playerVelocity=0, roadOffset = 0, spawnTimer = 0, elapsed = 0;
int score = 0, best = 0;
bool gameOver = false, started = false;
bool settingsOpen=false, soundEnabled=true, kazakh=false;
int difficulty=1;
float volume=.35f;
ALCdevice* audioDevice=nullptr; ALCcontext* audioContext=nullptr; ALuint audioSource=0,audioBuffer=0;
std::vector<Car> traffic;

void text(float x,float y,const std::string& s,float scale);
void kzText(float x,float y,const std::string& s,float scale);
void label(float x,float y,const std::string& en,const std::string& kz,float scale=1){if(kazakh)kzText(x,y,kz,scale);else text(x,y,en,scale);}
void playTone(float frequency,float seconds){
    if(!soundEnabled||!audioSource)return;
    constexpr int rate=22050;int count=static_cast<int>(rate*seconds);std::vector<short> samples(count);
    for(int i=0;i<count;i++){float envelope=std::min(1.0f,i/300.0f)*std::max(0.0f,1.0f-i/static_cast<float>(count));samples[i]=static_cast<short>(std::sin(6.2831853f*frequency*i/rate)*envelope*volume*24000);}
    alSourceStop(audioSource);alBufferData(audioBuffer,AL_FORMAT_MONO16,samples.data(),count*sizeof(short),rate);alSourcei(audioSource,AL_BUFFER,audioBuffer);alSourcePlay(audioSource);
}

void color(float r,float g,float b) { glColor3f(r,g,b); }
void rect(float x,float y,float w,float h,float r,float g,float b) {
    color(r,g,b); glBegin(GL_QUADS); glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h); glEnd();
}
void circle(float x,float y,float radius,float r,float g,float b) {
    color(r,g,b); glBegin(GL_TRIANGLE_FAN); glVertex2f(x,y);
    for(int i=0;i<=24;i++){float a=6.2831853f*i/24;glVertex2f(x+std::cos(a)*radius,y+std::sin(a)*radius);} glEnd();
}
void text(float x,float y,const std::string& s,float scale) {
    // Tiny built-in 5x7 font keeps the game dependency-free beyond GLFW/OpenGL.
    static const char* glyphs[]={
      "01110/10001/10011/10101/11001/10001/01110","00100/01100/00100/00100/00100/00100/01110",
      "01110/10001/00001/00010/00100/01000/11111","11110/00001/00001/00110/00001/00001/11110",
      "00010/00110/01010/10010/11111/00010/00010","11111/10000/10000/11110/00001/00001/11110",
      "01110/10000/10000/11110/10001/10001/01110","11111/00001/00010/00100/01000/01000/01000",
      "01110/10001/10001/01110/10001/10001/01110","01110/10001/10001/01111/00001/00001/01110",
      "01110/10001/10001/11111/10001/10001/10001","11110/10001/10001/11110/10001/10001/11110",
      "01111/10000/10000/10000/10000/10000/01111","11110/10001/10001/10001/10001/10001/11110",
      "11111/10000/10000/11110/10000/10000/11111","11111/10000/10000/11110/10000/10000/10000",
      "01111/10000/10000/10111/10001/10001/01111","10001/10001/10001/11111/10001/10001/10001",
      "01110/00100/00100/00100/00100/00100/01110","00111/00010/00010/00010/10010/10010/01100",
      "10001/10010/10100/11000/10100/10010/10001","10000/10000/10000/10000/10000/10000/11111",
      "10001/11011/10101/10101/10001/10001/10001","10001/11001/10101/10011/10001/10001/10001",
      "01110/10001/10001/10001/10001/10001/01110","11110/10001/10001/11110/10000/10000/10000",
      "01110/10001/10001/10001/10101/10010/01101","11110/10001/10001/11110/10100/10010/10001",
      "01111/10000/10000/01110/00001/00001/11110","11111/00100/00100/00100/00100/00100/00100",
      "10001/10001/10001/10001/10001/10001/01110","10001/10001/10001/10001/10001/01010/00100",
      "10001/10001/10001/10101/10101/10101/01010","10001/10001/01010/00100/01010/10001/10001",
      "10001/10001/01010/00100/00100/00100/00100","11111/00001/00010/00100/01000/10000/11111",
      "01110/10001/10011/10101/11001/10001/01110","00100/01100/00100/00100/00100/00100/01110",
      "01110/10001/00001/00010/00100/01000/11111","11110/00001/00001/00110/00001/00001/11110",
      "00010/00110/01010/10010/11111/00010/00010","11111/10000/10000/11110/00001/00001/11110",
      "01110/10000/10000/11110/10001/10001/01110","11111/00001/00010/00100/01000/01000/01000",
      "01110/10001/10001/01111/00001/00010/11100","01110/10001/10001/01111/00001/00001/01110"};
    color(1,1,1);
    for(char c:s){if(c==' '){x+=6*scale;continue;} int id=-1;
      if(c>='A'&&c<='Z')id=c-'A'; else if(c>='0'&&c<='9')id=26+c-'0';
      if(id>=0&&id<36){std::string g=glyphs[id];int row=0,col=0;for(char bit:g){if(bit=='/'){row++;col=0;}else{if(bit=='1')rect(x+col*scale,y-row*scale,scale,scale,1,1,1);col++;}}}
      x+=6*scale;
    }
}
void kzText(float x,float y,const std::string& s,float scale) {
    // UTF-8 additions for Kazakh Cyrillic letters used by the interface.
    struct Extra { const char* utf8; const char* rows; };
    static const Extra extra[]={
      {"А","01110/10001/10001/11111/10001/10001/10001"},{"Ә","01110/10000/11110/10001/11111/10001/01110"},
      {"Ғ","11111/10000/10000/11110/10000/10000/10000"},{"Қ","10001/10010/10100/11000/10100/10010/10001"},
      {"Ң","10001/11001/10101/10011/10001/10001/10011"},{"Ө","01110/10001/10001/10101/10001/10001/01110"},
      {"Ұ","01010/01010/01010/01010/01010/01010/11111"},{"Ү","10001/10001/01010/00100/00100/00100/00100"},
      {"Һ","10000/10000/11110/10001/10001/10001/10001"},{"І","00100/00000/01100/00100/00100/00100/01110"},
      {"і","00100/00000/00100/00100/00100/00100/00100"},{"ә","00000/01110/00001/01111/10001/10011/01101"},
      {"ғ","00111/00100/11110/00100/00100/00100/00100"},{"қ","10000/10010/10100/11000/10100/10010/10001"},
      {"ң","00000/11010/10101/10011/10001/10001/10011"},{"ө","00000/01110/10001/10101/10001/10001/01110"},
      {"ұ","00000/01010/01010/01010/01010/01010/11111"},{"ү","00000/10001/10001/01010/00100/00100/00100"},
      {"һ","10000/10000/11110/10001/10001/10001/10001"},{"А","01110/10001/10001/11111/10001/10001/10001"},
      {"Б","11111/10000/10000/11110/10001/10001/11110"},{"В","11110/10001/10001/11110/10001/10001/11110"},
      {"Д","00111/00101/00101/01001/01001/11111/10001"},{"Е","11111/10000/10000/11110/10000/10000/11111"},
      {"Ж","10101/10101/01110/00100/01110/10101/10101"},{"З","11110/00001/00001/01110/00001/00001/11110"},
      {"И","10001/10011/10101/10101/11001/10001/10001"},{"Й","00100/01010/00000/10001/10011/10101/11001"},
      {"Л","00111/01001/01001/01001/01001/10001/10001"},{"М","10001/11011/10101/10101/10001/10001/10001"},
      {"Н","10001/10001/10001/11111/10001/10001/10001"},{"О","01110/10001/10001/10001/10001/10001/01110"},
      {"П","11111/10001/10001/10001/10001/10001/10001"},{"Р","11110/10001/10001/11110/10000/10000/10000"},
      {"С","01111/10000/10000/10000/10000/10000/01111"},{"Т","11111/00100/00100/00100/00100/00100/00100"},
      {"У","10001/10001/01010/00100/00100/01000/10000"},{"Ф","00100/01110/10101/10101/01110/00100/00100"},
      {"Х","10001/01010/00100/00100/00100/01010/10001"},{"Ы","10001/10001/10001/11101/10011/10011/11101"},
      {"Э","01110/10001/00001/00111/00001/10001/01110"},{"Ю","10010/10101/10101/11101/10101/10101/10010"},
      {"Я","01111/10001/10001/01111/00101/01001/10001"},
      {"а","00000/01110/00001/01111/10001/10011/01101"},{"б","00111/01000/11110/10001/10001/10001/01110"},
      {"в","00000/11110/10001/11110/10001/10001/11110"},{"д","00000/00111/00101/01001/01001/11111/10001"},
      {"е","00000/01110/10001/11111/10000/10001/01110"},{"ж","00000/10101/10101/01110/00100/01110/10101"},
      {"з","00000/11110/00001/00110/00001/00001/11110"},{"и","00000/10001/10011/10101/11001/10001/10001"},
      {"й","00100/01010/00000/10001/10011/10101/11001"},{"л","00000/00111/01001/01001/01001/10001/10001"},
      {"м","00000/10001/11011/10101/10001/10001/10001"},{"н","00000/10001/10001/11111/10001/10001/10001"},
      {"о","00000/01110/10001/10001/10001/10001/01110"},{"п","00000/11111/10001/10001/10001/10001/10001"},
      {"р","00000/11110/10001/10001/11110/10000/10000"},{"с","00000/01111/10000/10000/10000/10000/01111"},
      {"т","00000/11111/00100/00100/00100/00100/00100"},{"у","00000/10001/10001/01010/00100/01000/10000"},
      {"ф","00100/01110/10101/10101/01110/00100/00100"},{"х","00000/10001/01010/00100/00100/01010/10001"},
      {"ы","00000/10001/10001/11101/10011/10011/11101"},{"э","00000/01110/10001/00111/00001/10001/01110"},
      {"ю","00000/10010/10101/10101/11101/10101/10010"},{"я","00000/01111/10001/10001/01111/00101/10001"}
    };
    color(1,1,1);
    for(size_t i=0;i<s.size();){
      if(s[i]==' '){x+=6*scale;i++;continue;}
      size_t len=(static_cast<unsigned char>(s[i])<128)?1:(static_cast<unsigned char>(s[i])<224?2:(static_cast<unsigned char>(s[i])<240?3:4));
      std::string ch=s.substr(i,len);i+=len;
      if(len==1){text(x,y,ch,scale);x+=6*scale;continue;}
      std::string g;
      for(const auto& e:extra)if(ch==e.utf8){g=e.rows;break;}
      if(g.empty()){x+=6*scale;continue;}int row=0,col=0;
      for(char bit:g){if(bit=='/'){row++;col=0;}else{if(bit=='1')rect(x+col*scale,y-row*scale,scale,scale,1,1,1);col++;}}
      x+=6*scale;
    }
}
void car(float x,float y,float r,float g,float b,bool player=false) {
    // Shadow, tires, body, windows and headlights.
    rect(x-19,y-28,40,68,0.04f,0.05f,0.06f);
    rect(x-22,y-24,7,19,0.08f,0.08f,0.09f); rect(x+15,y-24,7,19,0.08f,0.08f,0.09f);
    rect(x-22,y+12,7,18,0.08f,0.08f,0.09f); rect(x+15,y+12,7,18,0.08f,0.08f,0.09f);
    rect(x-16,y-34,32,72,r,g,b); rect(x-12,y-24,24,16,0.12f,0.19f,0.22f);
    rect(x-11,y-22,22,11,0.2f,0.55f,0.66f); rect(x-10,y+13,20,13,0.12f,0.19f,0.22f);
    rect(x-12,y+30,7,4,1,0.9f,0.5f); rect(x+5,y+30,7,4,1,0.9f,0.5f);
    if(player){rect(x-6,y-3,12,3,0.93f,0.72f,0.15f);}
}
void reset() { playerX=240;playerVelocity=0;score=0;elapsed=0;spawnTimer=0;traffic.clear();gameOver=false;started=true; }
void key(GLFWwindow*,int key,int,int action,int) {
    if(action!=GLFW_PRESS&&action!=GLFW_REPEAT)return;
    if(key==GLFW_KEY_ESCAPE){if(settingsOpen)settingsOpen=false;else glfwSetWindowShouldClose(window,GLFW_TRUE);return;}
    if(key==GLFW_KEY_S&&(!started||gameOver)){settingsOpen=!settingsOpen;return;}
    if(settingsOpen){
      if(key==GLFW_KEY_L)kazakh=!kazakh;
      if(key==GLFW_KEY_M)soundEnabled=!soundEnabled;
      if(key==GLFW_KEY_C)difficulty=(difficulty+1)%3;
      if(key==GLFW_KEY_EQUAL||key==GLFW_KEY_KP_ADD)volume=std::min(1.0f,volume+.1f);
      if(key==GLFW_KEY_MINUS||key==GLFW_KEY_KP_SUBTRACT)volume=std::max(0.0f,volume-.1f);
      if(key==GLFW_KEY_ENTER)settingsOpen=false;
      return;
    }
    if((key==GLFW_KEY_ENTER||key==GLFW_KEY_SPACE)&&(gameOver||!started)){reset();playTone(660,.12f);}
}
void frame() {
    glClearColor(.08f,.32f,.08f,1);glClear(GL_COLOR_BUFFER_BIT);
    rect(72,0,336,H,.16f,.17f,.18f);
    rect(86,0,5,H,.97f,.65f,.05f);rect(399,0,5,H,.97f,.65f,.05f);
    rect(94,0,2,H,.98f,.82f,.16f);rect(394,0,2,H,.98f,.82f,.16f);
    float dash=std::fmod(roadOffset,96.0f);
    for(float y=H-dash;y>-96;y-=96){rect(190,y,5,48,.91f,.92f,.92f);rect(285,y,5,48,.91f,.92f,.92f);}
    // roadside grass and simple shrubs
    for(int i=0;i<8;i++){float y=std::fmod(i*101.0f-roadOffset*.55f,H);if(y<0)y+=H;circle(35,y,11,.12f,.55f,.08f);circle(444,y+25,10,.14f,.59f,.09f);}
    for(const auto& c:traffic)car(c.x,c.y,.95f,.29f,.11f);
    car(playerX,115,.10f,.42f,.96f,true);
    rect(0,0,W,58,.04f,.05f,.06f);label(18,39,"SCORE","ҰПАЙ",1.6f);text(112,39,std::to_string(score),1.8f);
    label(282,39,"BEST","ҮЗДІК",1.3f);text(393,39,std::to_string(std::max(best,score)),1.5f);
    if(settingsOpen){rect(33,170,414,380,.02f,.03f,.04f);rect(37,174,406,342,.1f,.11f,.12f);
      label(153,480,"SETTINGS","БАПТАУЛАР",2.0f);
      label(91,430,kazakh?"LANGUAGE (L): KAZAKH":"LANGUAGE (L): ENGLISH",kazakh?"ТІЛ (L): ҚАЗАҚША":"ТІЛ (L): АҒЫЛШЫНША",1.05f);
      label(91,390,soundEnabled?"SOUND (M): ON":"SOUND (M): OFF",soundEnabled?"ДЫБЫС (M): ҚОСУЛЫ":"ДЫБЫС (M): ӨШІРУЛІ",1.2f);
      label(91,350,"DIFFICULTY (C):","ҚИЫНДЫҚ (C):",1.2f);
      label(250,350,difficulty==0?"EASY":difficulty==1?"NORMAL":"HARD",difficulty==0?"ОҢАЙ":difficulty==1?"ОРТАША":"ҚИЫН",1.2f);
      label(91,310,"VOLUME (- / +):","ДЫБЫС ДЕҢГЕЙІ (- / +):",1.2f);
      rect(91,282,270,10,.22f,.24f,.25f);rect(91,282,270*volume,10,.12f,.52f,.97f);
      label(91,240,"ENTER / ESC: BACK","ENTER / ESC: ҚАЙТУ",1.2f);return;}
    if(!started||gameOver){rect(33,245,414,230,.02f,.03f,.04f);rect(37,249,406,222,.1f,.11f,.12f);
      if(gameOver){label(137,413,"GAME OVER","ОЙЫН БІТТІ",2.0f);label(150,362,"SCORE","ҰПАЙ",1.7f);text(225,362,std::to_string(score),1.7f);label(112,310,"RESTART: ENTER","ҚАЙТА БАСТАУ: ENTER",1.2f);}
      else{label(101,413,"CAR RACING 2D","ЖАРЫС ОЙЫНЫ 2D",1.8f);label(119,363,"STEER: A / D","БАСҚАРУ: A / D",1.5f);label(128,326,"AVOID TRAFFIC","КӨЛІКТЕРДЕН ҚАШ",1.4f);label(143,294,"ENTER: START","ENTER: БАСТАУ",1.3f);label(156,265,"S: SETTINGS","S: БАПТАУ",1.3f);}}
    if(started&&!gameOver)label(128,22,"A / D OR ARROWS: STEER","A / D НЕМЕСЕ БАҒЫТ ПЕРНЕЛЕРІ",.9f);
}
}
int main(){
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
    glfwWindowHint(GLFW_SAMPLES,4);
    window=glfwCreateWindow(480,720,"Car Racing 2D",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}glfwMakeContextCurrent(window);glfwSwapInterval(1);glfwSetKeyCallback(window,key);
    audioDevice=alcOpenDevice(nullptr);if(audioDevice){audioContext=alcCreateContext(audioDevice,nullptr);if(audioContext&&alcMakeContextCurrent(audioContext)){alGenBuffers(1,&audioBuffer);alGenSources(1,&audioSource);}}
    glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,W,0,H,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    double last=glfwGetTime();
    while(!glfwWindowShouldClose(window)){
      double now=glfwGetTime();float dt=static_cast<float>(std::min(now-last,0.05));last=now;
      if(started&&!gameOver){
        float steer=0;
        if(glfwGetKey(window,GLFW_KEY_LEFT)==GLFW_PRESS||glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)steer-=1;
        if(glfwGetKey(window,GLFW_KEY_RIGHT)==GLFW_PRESS||glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)steer+=1;
        playerVelocity+=steer*900.0f*dt;
        playerVelocity*=std::exp(-5.5f*dt);
        playerVelocity=std::clamp(playerVelocity,-340.0f,340.0f);
        playerX+=playerVelocity*dt;
        if(playerX<112.0f){playerX=112.0f;playerVelocity=std::max(playerVelocity,0.0f);}
        if(playerX>368.0f){playerX=368.0f;playerVelocity=std::min(playerVelocity,0.0f);}
        elapsed+=dt;roadOffset+=dt*(235+std::min(elapsed*7,200.0f));spawnTimer+=dt;
        const float diffFactor=difficulty==0?.78f:difficulty==1?1.0f:1.24f;
        if(spawnTimer>std::max(.38f,(1.05f-elapsed*.012f)/diffFactor)){spawnTimer=0;int lane=std::rand()%3;float x=144+lane*96;
          bool clear=true;for(const auto& c:traffic)if(c.lane==lane&&c.y>H-150)clear=false;
          if(clear)traffic.push_back({x,H+50,(220+std::min(elapsed*6,190.0f))*diffFactor,lane});}
        for(auto& c:traffic)c.y-=c.speed*dt;
        for(auto it=traffic.begin();it!=traffic.end();){if(it->y<-50){score++;best=std::max(best,score);playTone(780,.07f);it=traffic.erase(it);}else{
          if(std::abs(it->x-playerX)<30&&std::abs(it->y-115)<62){gameOver=true;playerVelocity=0;playTone(130,.35f);}++it;}}
      }
      int ww,wh;glfwGetFramebufferSize(window,&ww,&wh);glViewport(0,0,ww,wh);glMatrixMode(GL_PROJECTION);glLoadIdentity();
      float aspect=static_cast<float>(ww)/std::max(wh,1);float viewH=H,viewW=viewH*aspect;
      if(viewW<W){viewW=W;viewH=viewW/aspect;}glOrtho((W-viewW)/2,(W+viewW)/2,(H-viewH)/2,(H+viewH)/2,-1,1);
      glEnable(GL_MULTISAMPLE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glMatrixMode(GL_MODELVIEW);glLoadIdentity();frame();glfwSwapBuffers(window);glfwPollEvents();
    }
    if(audioSource)alDeleteSources(1,&audioSource);if(audioBuffer)alDeleteBuffers(1,&audioBuffer);
    if(audioContext){alcMakeContextCurrent(nullptr);alcDestroyContext(audioContext);}if(audioDevice)alcCloseDevice(audioDevice);
    glfwDestroyWindow(window);glfwTerminate();return 0;
}
