//Khai bao thu vien
#include <bits/stdc++.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

//khai bao cac thong so co dinh
const int SCREEN_HEIGHT = 600 ;
const int SCREEN_WIDTH = 1200;
const int FRAME = 60 ;
const int DINO_WIDTH = 64;
const int DINO_HEIGHT = 64;
const int COLOR_KEY_R = 167;
const int COLOR_KEY_G = 175 ;
const int COLOR_KEY_B = 180 ;
const int RENDER_DRAW_COLOR = 255 ;

//khai bao cac bien sdl
SDL_Window* gwindow  = nullptr ;
SDL_Renderer* grender = nullptr ;
SDL_Texture* gDinoTexture = nullptr ;
SDL_Texture* gBackgroundTexture = nullptr ;
SDL_Texture* gObstacleTexture1 = nullptr ;
SDL_Texture* gObstacleTexture2 = nullptr ;
SDL_Texture* gObstacleTexture3 = nullptr ;
SDL_Event gevent ;

//am thanh
Mix_Music* gMenuMusic = nullptr ;
Mix_Chunk* gJump = nullptr ;
Mix_Chunk* gDead = nullptr ;
Mix_Chunk* gPoint = nullptr ;
Mix_Chunk* gClick = nullptr ;

//cac bien bool xu ly
bool isRunning = true ;
bool loadMenu = true ;
bool isStarting = false ;
bool isHelping = false ;
bool isQuitting = false ;
bool endGame = false;
bool playingMusic = true ;

//text
TTF_Font* font_time = nullptr;

//tao lop object
class object
{
protected :
    //luu kich thuoc va vi tri hien thi cua buc anh
    SDL_Rect rect_ ;
    //luu thong so tam anh
    SDL_Texture* objectTexture;
public:
    //toc do di chuyen len xuong
    float x_val, y_val ;
    object()
    {
        rect_.x = 0 ;
        rect_.y = 0 ;
        rect_.w = 0 ;
        rect_.h = 0 ;
        objectTexture = nullptr ;
        x_val = 0 ;
        y_val = 0 ;

    };
    ~object(){};
    //phuong thuc de cai dat vi tri
    void setPos(const int& xpos, const int& ypos){rect_.x = xpos ; rect_.y = ypos ;}
    //phuong thuc cai dat size cho tam anh
    void setSize(const int& width, const int& height){rect_.w = width ; rect_.h = height;}
    //phuong thuc lay kich thuoc tam anh
    SDL_Rect getRect() const{return rect_ ;}
    //phuong thuc load texture tam anh
    SDL_Texture* LoadTexture(std::string path)
    {
        //DOC TAM ANH THANH SURFACE
        SDL_Surface* loadsurface = IMG_Load(path.c_str()) ;
        if(loadsurface != NULL)
        {
            //TRANPARENCE BACKGROUND
            SDL_SetColorKey(loadsurface, SDL_TRUE,SDL_MapRGB(loadsurface->format ,COLOR_KEY_R,COLOR_KEY_G,COLOR_KEY_B)) ;

            //CHUYEN SURFACE THANH TEXTURE LUU DU TAT CA THONG TIN VE TAM ANH
            objectTexture = SDL_CreateTextureFromSurface(grender, loadsurface) ;
            if(objectTexture != nullptr)
            {
                rect_.w = loadsurface->w ;
                rect_.h = loadsurface->h ;
            }

            //FREE(XOA) SURFACE
            SDL_FreeSurface(loadsurface) ;
        }
        return objectTexture;
    }
    // phuong thuc de ve doi tuong
    void draw(SDL_Renderer* render,SDL_Texture* texture,const SDL_Rect* srcRect)
    {
        SDL_Rect renderquad = {rect_.x,rect_.y,rect_.w,rect_.h} ;
        //DAY DOI TUONG LEN MAN HINH
        SDL_RenderCopy(render,texture,srcRect,&renderquad) ;
    }
    // phuong thuc checkfocus de xu ly button
    bool checkFocus(const int& x , const int& y)
    {
        if(x >= rect_.x && x <= rect_.x + rect_.w&&
           y >= rect_.y && y <= rect_.y + rect_.h)
            return true ;
        return false ;

    }
    //phuong thuc giai phong texture
    void Free(SDL_Texture* texture)
    {
        SDL_DestroyTexture(objectTexture);
        objectTexture = nullptr ;
        SDL_DestroyTexture(texture) ;
        texture = nullptr ;
    }
};

//tao lop main object: dino ke thua tu lop object
class Dino:public object
{
private :
    //trang thai doi tuong
    enum DinoState
    {
        DEAD = 0 ,
        RUN = 1 ,
        JUMP = 2 ,
        DOWN = 3,
        FALL = 4
    };
    float gravity, jumpVel ;
    int maxHeight,minHeight,x_pos,y_pos;
    int state ;
    int frameDuration ;
    bool onGround ;
public:
    Dino()
    {
        gravity = 1.6  ;
        jumpVel = -1.4;
        maxHeight = SCREEN_HEIGHT - DINO_HEIGHT - 200 ;
        minHeight = SCREEN_HEIGHT - DINO_HEIGHT - 10 ;
        x_pos = 40 ;
        y_pos = minHeight ;
        state = 1 ;
        onGround = true ;
        frameDuration = 100 ;

    }
    ~Dino(){} ;
    //hien thi doi tuong dino
    void Show()
    {
        //set lại vị trí hiển thị dino
        rect_.x = x_pos ;
        rect_.y = y_pos ;

        int frameNum = 2 ;
        SDL_Rect FrameRun[frameNum] ;
        FrameRun[0] = {0,0,DINO_WIDTH,DINO_HEIGHT} ;
        FrameRun[1] = {DINO_WIDTH,0,DINO_WIDTH,DINO_HEIGHT} ;

        SDL_Rect FrameDown[frameNum] ;
        FrameDown[0] = {0,0,DINO_WIDTH,32} ;
        FrameDown[1] = {DINO_WIDTH,0,DINO_WIDTH,32} ;

        int currentframe = (SDL_GetTicks()/frameDuration)%frameNum ;
        switch(state)
        {
        case DEAD:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Dead.png");
            draw(grender, gDinoTexture,NULL);
            Mix_PlayChannel(-1,gDead,0) ;

            break;
         case RUN:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Run.png") ;
            rect_.w = DINO_WIDTH ;
            rect_.h = DINO_HEIGHT ;
            draw(grender, gDinoTexture,&FrameRun[currentframe]);
            break ;
        case JUMP:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Jump.png");
            y_pos += jumpVel ;
            if(y_pos < maxHeight )
            {
                state = FALL ;
                y_pos = maxHeight;
            }
            if(y_pos == minHeight+jumpVel)
            {
                Mix_PlayChannel(-1,gJump,0) ;
            }
            draw(grender,gDinoTexture,nullptr) ;
            break ;
        case DOWN:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Down.png") ;
            //if(x_pos<minHeight)
            //{
                //state = FALL ;
            //}
            setPos(40, minHeight+32) ;
            draw(grender, gDinoTexture,&FrameDown[currentframe]);
            break ;
        case FALL :
            gDinoTexture = LoadTexture("Image//Dino//Dino_Jump.png");
            y_pos += gravity ;
            if(y_pos > minHeight)
            {
                y_pos = minHeight ;
                state = RUN ;
            }
            draw(grender,gDinoTexture,nullptr);
            break ;
        }
        Free(gDinoTexture) ;
    }
    //phuong thuc xu ly su kien voi dino
    void HandleEvent(SDL_Event event)
    {
           if(event.type==SDL_KEYDOWN)
        {
            if((event.key.keysym.sym == SDLK_SPACE|| event.key.keysym.sym ==SDLK_UP) && state == RUN)
                state = JUMP ;
            else if(event.key.keysym.sym ==SDLK_DOWN )
            {
                if(state == JUMP)
                {
                    state = JUMP ;
                }
                else if(state == FALL)
                {
                    state == FALL ;
                }
                else
                {
                    state = DOWN ;

                }
            }

        }
        else if (event.type==SDL_KEYUP)
        {
            if((event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym ==SDLK_UP) && state == RUN)
            {
                state = JUMP ;
            }
            else if(event.key.keysym.sym ==SDLK_DOWN)
            {
                if(state == JUMP)
                {
                    state = JUMP ;
                }
                else if(state == FALL)
                {
                    state == FALL ;
                }
                else
                {
                    state = DOWN ;
                    state = RUN ;
                }
            }
        }

    }
    //phuong thuc check va cham voi chuong ngai vat
    bool checkCollision(const SDL_Rect& obstacleRect)
    {
        int dinoLeft = rect_.x;
        int dinoRight = dinoLeft + rect_.w;
        int dinoTop = rect_.y;
        int dinoBottom = rect_.y + rect_.h;

        int obstacleLeft = obstacleRect.x;
        int obstacleRight = obstacleRect.x + obstacleRect.w;
        int obstacleTop = obstacleRect.y;
        int obstacleBottom = obstacleRect.y + obstacleRect.w;

        if (dinoRight > obstacleLeft+20 && dinoLeft < obstacleRight-20 &&
        dinoBottom > obstacleTop+20 && dinoTop < obstacleBottom-20)
        {
            state = DEAD ;
            return true; // Có va chạm
        }
        if (dinoRight > obstacleLeft+20 && dinoLeft < obstacleRight-20 &&
        dinoBottom > obstacleTop+20 && dinoTop < obstacleBottom-20)
        {
            state = DEAD ;
            return true;
        }

        return false ;
    }
    //phuong thuc kiem tra xem co dang o mat dat khong
    bool checkOnGround()
    {
        if(rect_.y == minHeight)
        {
            return true ;
        }
        else return false ;
    }
    //phuong thuc xet duration
    void setDuration(const int& x){frameDuration = x ;}
    //phuong thuc cai dat trang thai dino
    void setState(const int& x){state = x;}
    //phuong thuc cai dat xpos ypos
    void setXYPos(const int& newXPos , const int& newYPos){x_pos = newXPos; y_pos = newYPos ;}
    //phuong thuc cai dat jumpVel, gravity
    void set_jumpVel_gravity(const int& jumpVelNew, const int& gravityNew){jumpVel = jumpVelNew; gravity = gravityNew ;}
};

//tao lop obstacle ke thua tu lop object
class Obstacle: public object
{
private:

public:
    Obstacle()
    {
        rect_.x =SCREEN_WIDTH;
        rect_.y =0;
        rect_.w = 0 ;
        rect_.h = 0 ;
        x_val = -1 ;
        y_val = 0 ;
    }
    ~Obstacle(){;}
    //phuong thuc xu ly di chuyen
    void HandleMove(const int& x_pos, const int& y_pos)
    {
        rect_.x += x_val ;
        if(rect_.x < -rect_.w)
        {
            setPos(x_pos,y_pos);
        }
    }
    void HandleInputAction(SDL_Event event);
    //phuong thuc cai dat toc do di chuyen obstacle
    void setXVal(const int& newXVal){x_val = newXVal ; }

};

//lop text
class Text
{
public:
    enum ColorText
    {
        RED = 0 ,
        WHITE = 1 ,
        BLACK = 2,
        YELLOW = 3,
        CYAN = 4
    };
    Text()
    {
        colorText.r = 255 ;
        colorText.g = 255 ;
        colorText.b = 255 ;
        textTexture =nullptr ;
    }
    ~Text(){}
    //phuong thuc load texture cua text
    void LoadTextTexture(TTF_Font* font, SDL_Renderer* render)
    {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, stringText.c_str(), colorText);
        if(textSurface!=NULL)
        {
            textTexture = SDL_CreateTextureFromSurface(render,textSurface) ;
            width = textSurface->w  ;
            height = textSurface->h ;
            SDL_FreeSurface(textSurface) ;
        }
        else std::cout << "loi textSurface";
    }
    //phuong thuc giai phong texture cua text
    void free()
    {
        SDL_DestroyTexture(textTexture) ;
        textTexture = nullptr ;
    }
    //phuong thuc cai dat mau cho chu
    void setColor(int type)
    {
        if(type == RED)
        {
            SDL_Color color = {255,0,0} ;
            colorText = color ;
        }
        else if(type == WHITE)
        {
            SDL_Color color = {255,255,255} ;
            colorText = color ;
        }
        else if(type == BLACK)
        {
            SDL_Color color = {0,0,0} ;
            colorText = color ;
        }
        else if(type == YELLOW)
        {
            SDL_Color color = {243, 222, 8} ;
            colorText = color ;
        }
        else if(type == CYAN)
        {
            SDL_Color color = {5, 221, 230};
            colorText = color ;
        }
    }
    //phuong thuc cai dat chuoi text
    void setText(const std::string& text){stringText= text ; }
    //phuong thuc show text
    void showText(SDL_Renderer* render, int xp, int yp, double tiltAngle)
    {
        SDL_Rect rectText = {xp,yp,width,height} ;
        SDL_RenderCopyEx(render,textTexture,nullptr,&rectText,tiltAngle,nullptr,SDL_FLIP_NONE) ;
        free();

    }
private:
    std::string stringText ;
    SDL_Color colorText ;
    SDL_Texture* textTexture ;
    int width ;
    int height ;
};

//ham khoi tao
bool Init()
{
    //tao bien kiem tra ,khoi tao window, render
    bool success = true ;
    //khoi tao
    int ret = SDL_Init(SDL_INIT_VIDEO) ;
    if(ret < 0 )
        success = false ;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") ;
    // tao window
    gwindow = SDL_CreateWindow("CHROME DINO GAME",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN) ;
    // TAOWINDOW THAT BAI
    if(gwindow == NULL)
    {
        success = false ;
    }
    //NEU THANH CONG
    else
    {
        // TAO RENDERER CHO WINDOW
        grender = SDL_CreateRenderer(gwindow,-1,SDL_RENDERER_ACCELERATED) ;
        //NEU RENDER LOI
        if(grender == NULL)
            success = false ;
        //TAO RENDER THANH CONG
        else
        {
            // KHOI TAO RENDERER COLOR
            SDL_SetRenderDrawColor(grender,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR) ;

            //KHOI TAO PNG LOADING, TAO CO
            int imgFlags = IMG_INIT_PNG ;
            if(!(IMG_Init(imgFlags) && imgFlags))
            {
                success = false ;

            }

        }
    }

    //load am thanh
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,2,4096) == -1)
    {
        std::cout << "Lỗi âm thanh" ;
        success = false ;
    }
    gClick = Mix_LoadWAV("Sound//Click.wav") ;
    gDead = Mix_LoadWAV("Sound//Dead.wav") ;
    gJump = Mix_LoadWAV("Sound//Jump.wav") ;
    gPoint = Mix_LoadWAV("Sound//Point.wav");
    gMenuMusic = Mix_LoadMUS("Sound//Clown.wav") ;
    if(gDead == NULL || gJump == NULL||gMenuMusic == NULL || gClick==NULL)
    {
        std::cout << "loi load file am thanh" ;
        success = false ;
    }

    //load text
    if(TTF_Init()== -1)
    {
        std::cout << "loi TTF_Init()" ;
        success = false ;
    }
    else
    {
        font_time = TTF_OpenFont("font//dlxfont_.ttf",20) ;
        if(font_time == nullptr)
        {
            std::cout << "loi font_time"  ;
            success = false ;
        }
    }

    return success ;
}

//ham giai phong bo nho
void close()
{
    SDL_DestroyWindow(gwindow) ;
    gwindow = nullptr;
    SDL_DestroyRenderer(grender);
    grender = nullptr;
    SDL_DestroyTexture(gBackgroundTexture) ;
    gBackgroundTexture = nullptr ;
    SDL_DestroyTexture(gDinoTexture) ;
    gDinoTexture = nullptr ;

    SDL_DestroyTexture(gObstacleTexture1) ;
    gObstacleTexture1 = nullptr ;

    SDL_DestroyTexture(gObstacleTexture2) ;
    gObstacleTexture2 = nullptr ;

    SDL_DestroyTexture(gObstacleTexture3) ;
    gObstacleTexture3 = nullptr ;

    Mix_FreeChunk(gClick);
    gClick = nullptr ;
    Mix_FreeChunk(gJump) ;
    gJump = nullptr ;
    Mix_FreeChunk(gDead) ;
    gDead = nullptr ;
    Mix_FreeMusic(gMenuMusic);
    gMenuMusic = nullptr ;
    IMG_Quit() ;
    SDL_QUIT;
}

//ham main
int main(int argc, char* argv[])
{
    if(Init()==false)
        std::cout << "Khoi tao khong thanh cong" ;
    //Background
    object Background ;
    gBackgroundTexture = Background.LoadTexture("Image//Background//Background.png") ;
    //Dino
    Dino dino ;

    //Obstacle
    Obstacle obstacle1;
    gObstacleTexture1 = obstacle1.LoadTexture("Image//Obstacles//Ob4.png") ;
    SDL_Rect obstacleRect1 = obstacle1.getRect() ;
    obstacle1.setPos(SCREEN_WIDTH+ 500,SCREEN_HEIGHT-obstacleRect1.h);


    Obstacle obstacle2 ;
    gObstacleTexture2 = obstacle2.LoadTexture("Image//Obstacles//Ob11.png") ;
    SDL_Rect obstacleRect2 = obstacle2.getRect() ;
    obstacle2.setPos(SCREEN_WIDTH + 1000,SCREEN_HEIGHT-obstacleRect2.h);

    Obstacle obstacle3 ;
    gObstacleTexture3 = obstacle3.LoadTexture("Image//Obstacles//Ob16.png") ;
    SDL_Rect obstacleRect3 = obstacle3.getRect() ;
    obstacle3.setPos(SCREEN_WIDTH+2000,SCREEN_HEIGHT-obstacleRect3.h);

    /*
    int randomFile = std::rand()%(16)+1 ;
    std::string fileName = "Ob"+std::to_string(randomFile) ;
    std::string filePath = directory+fileName+fileExtension ;
    //std::cout << filePath ;
    //gObstacleTexture = obstacle.LoadTexture(filePath) ;
    //SDL_Rect obstacleRect = obstacle.getRect();
    //obstacle.setPos(SCREEN_WIDTH, SCREEN_HEIGHT - obstacleRect.h - 10) ;
    */

    Obstacle airplane;
    SDL_Texture* airplaneTexture = nullptr ;
    airplane.setPos(SCREEN_WIDTH, SCREEN_HEIGHT*0.7) ;

    Text score_game ;
    score_game.setColor(Text::YELLOW);

    Text HighScore;
    HighScore.setColor(Text::YELLOW) ;
    HighScore.free() ;

    Uint32 timeValue;
    Uint32 scoreValue ;

    int numberPlay = 0 ;
    std::vector<int>scoreVector ;

    //Mix_PlayMusic(gMenuMusic,-1) ;
    while(isRunning)
    {
        SDL_SetRenderDrawColor(grender, RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR) ;
        SDL_RenderClear(grender) ;
        int mousePosx = gevent.motion.x ;
        int mousePosy = gevent.motion.y ;
        //Menu
        if(loadMenu)
        {
            timeValue = SDL_GetTicks();
            object Menu;
            SDL_Texture* menuTexture;
            menuTexture = Menu.LoadTexture("Image//Background//Menu1.png");
            Menu.draw(grender,menuTexture,nullptr) ;


            object buttonPlay ;
            SDL_Texture* buttonPlayTexture;
            buttonPlayTexture = buttonPlay.LoadTexture("Image//Background//Play1.png") ;
            buttonPlay.setPos(520,SCREEN_HEIGHT/2) ;

            object buttonHelp;
            SDL_Texture* buttonHelpTexture;
            buttonHelpTexture = buttonHelp.LoadTexture("Image//Background//Help1.png") ;
            buttonHelp.setPos(520,SCREEN_HEIGHT/2 + 100) ;

            object buttonExit ;
            SDL_Texture* buttonExitTexture = nullptr ;
            buttonExitTexture = buttonExit.LoadTexture("Image//Background//Quit1.png") ;
            buttonExit.setPos(520,SCREEN_HEIGHT/2+200) ;

            //SDL_Event eventMenu ;
            while(SDL_PollEvent(&gevent)!=0)
            {
                if(gevent.type == SDL_QUIT)
                {
                    loadMenu = false ;
                    isRunning = false ;
                }
                else if(gevent.type == SDL_MOUSEBUTTONUP||gevent.type==SDL_MOUSEBUTTONDOWN)
                {
                    std::cout << mousePosx << " " << mousePosy ;
                    if(buttonPlay.checkFocus(mousePosx,mousePosy) == true)
                    {
                        Mix_PlayChannel(-1, gClick,0) ;
                        isStarting = true;
                        loadMenu = false ;
                    }
                    else if(buttonHelp.checkFocus(mousePosx,mousePosy) == true)
                    {
                        Mix_PlayChannel(-1, gClick,0) ;
                        isHelping = true ;
                        loadMenu = false ;
                    }
                    else if(buttonExit.checkFocus(mousePosx,mousePosy) == true)
                    {
                        Mix_PlayChannel(-1, gClick,0) ;
                        isQuitting = true;
                        loadMenu = false ;
                    }
                }
            }
            //Mix_PlayMusic(gMenuMusic,-1) ;
            buttonPlay.draw(grender,buttonPlayTexture,nullptr);
            buttonHelp.draw(grender,buttonHelpTexture,nullptr);
            buttonExit.draw(grender,buttonExitTexture,nullptr);
            Menu.Free(menuTexture) ;
            buttonPlay.Free(buttonPlayTexture) ;
            buttonHelp.Free(buttonHelpTexture) ;
            buttonExit.Free(buttonExitTexture) ;
        }
        else if(isStarting)
        {
            airplaneTexture = airplane.LoadTexture("Image//Obstacles//Ob17.png");
            SDL_Rect airplaneRect = airplane.getRect();

            if(SDL_PollEvent(&gevent)!=0)
            {
                if(gevent.type == SDL_QUIT)
                {
                    loadMenu = true ;
                    isStarting = false ;
                }
                else if(gevent.type == SDL_KEYUP || gevent.type == SDL_KEYDOWN)
                {
                    if(gevent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        isStarting = false ;
                        loadMenu = true ;
                    }
                }
                dino.HandleEvent(gevent) ;
            }
            //BackGround
            if(scoreValue<= 100)
            {
                Background.x_val -= 0.4 ;
                dino.setDuration(140) ;
            }
            else if(scoreValue<=200)
            {
                Background.x_val -= 1 ;
                dino.setDuration(100) ;
                dino.set_jumpVel_gravity(-1.6, 2.2);
                airplane.setXVal(-3);
                obstacle1.setXVal(-2) ;
                obstacle2.setXVal(-2) ;
                obstacle3.setXVal(-2) ;
            }
            else if(scoreValue<=300)
            {
                Background.x_val -= 1.2 ;
                dino.setDuration(90) ;
                dino.set_jumpVel_gravity(-1.8,2.4);
                airplane.setXVal(-4);
                obstacle1.setXVal(-3) ;
                obstacle2.setXVal(-3) ;
                obstacle3.setXVal(-3) ;
            }
            else if(scoreValue<=400)
            {
                Background.x_val -= 1,4 ;
                dino.setDuration(80) ;
                dino.set_jumpVel_gravity(-1.9, 2.5);
                airplane.setXVal(-5);
                obstacle1.setXVal(-3) ;
                obstacle2.setXVal(-3) ;
                obstacle3.setXVal(-3) ;
            }
            else if(timeValue<=500)
            {
                Background.x_val -= 1.7 ;
                dino.setDuration(70) ;
                dino.set_jumpVel_gravity(-2, 2.6);
                airplane.setXVal(-6);
                obstacle1.setXVal(-3) ;
                obstacle2.setXVal(-3) ;
                obstacle3.setXVal(-3) ;
            }
            else if(scoreValue<=600)
            {
                Background.x_val -= 1.9 ;
                dino.setDuration(60) ;
                dino.set_jumpVel_gravity(-2.1, 3);
                airplane.setXVal(-6);
                obstacle1.setXVal(-4) ;
                obstacle2.setXVal(-4) ;
                obstacle3.setXVal(-4) ;
            }
            else if(scoreValue<=750)
            {
                Background.x_val -= 2 ;
                dino.setDuration(50) ;
                dino.set_jumpVel_gravity(-2.2, 3.2);
            }
            else
            {
                Background.x_val -= 3 ;
                dino.setDuration(40) ;
                dino.set_jumpVel_gravity(-3,5);
                airplane.setXVal(-7);
                obstacle1.setXVal(-5) ;
                obstacle2.setXVal(-5) ;
                obstacle3.setXVal(-5) ;
            }

            Background.setPos(Background.x_val, 0);
            Background.draw(grender , gBackgroundTexture,nullptr) ;
            Background.setPos(Background.x_val + SCREEN_WIDTH,0) ;
            Background.draw(grender, gBackgroundTexture,nullptr) ;
            if(Background.x_val <= -3*SCREEN_WIDTH)
            {
                Background.x_val=  0 ;
            }
            //Dino
            dino.Show() ;

            //obstacle

            int randomX  = 300 + std::rand()%(400-300+1) ;
            obstacle1.HandleMove(2700 ,SCREEN_HEIGHT - obstacleRect1.h) ;
            obstacle1.draw(grender,gObstacleTexture1,nullptr) ;

            obstacle2.HandleMove(1700 ,SCREEN_HEIGHT - obstacleRect2.h) ;
            obstacle2.draw(grender,gObstacleTexture2,nullptr) ;

            obstacle3.HandleMove(3700,SCREEN_HEIGHT - obstacleRect3.h) ;
            obstacle3.draw(grender,gObstacleTexture3,nullptr) ;

            //airplane
            int randomYAirplane = 450 + std::rand()%(470-450 + 1) ;
            airplane.HandleMove(1300,randomYAirplane);
            airplane.draw(grender, airplaneTexture,nullptr) ;
            airplane.Free(airplaneTexture) ;


            if(dino.checkCollision(obstacle1.getRect()) == true|| dino.checkCollision(obstacle2.getRect()) == true|| dino.checkCollision(obstacle3.getRect()) == true||  dino.checkCollision(airplane.getRect()) == true)
            {
                Mix_PlayChannel(-1, gDead,0) ;
                endGame = true ;
                isStarting = false ;

                dino.setXYPos(40,SCREEN_HEIGHT-DINO_HEIGHT-10) ;
                airplane.setPos(SCREEN_WIDTH,SCREEN_HEIGHT*0.7) ;

                obstacle1.setPos(SCREEN_WIDTH + 2*randomX,SCREEN_HEIGHT-obstacleRect1.h) ;
                obstacle2.setPos(SCREEN_WIDTH + 3*randomX,SCREEN_HEIGHT-obstacleRect2.h) ;
                obstacle3.setPos(SCREEN_WIDTH+ 4*randomX,SCREEN_HEIGHT-obstacleRect3.h) ;

                SDL_Delay(100) ;
            }
            std::string stringScore = "Score: " ;
            scoreValue = 10*SDL_GetTicks()/1000 - 10*timeValue/1000 ;
            if(scoreValue%100==0&&scoreValue!=0)
            {
                Mix_PlayChannel(-1,gPoint,0) ;
            }
            std::string scoreToString = std::to_string(scoreValue);
            stringScore+=scoreToString ;
            score_game.setText(stringScore);
            score_game.LoadTextTexture(font_time,grender) ;
            score_game.showText(grender,SCREEN_WIDTH- 220,15,0) ;
            score_game.free() ;
            if(numberPlay!=0)
            {
                std::string stringScoreMax = "High Score:" ;
                auto max_vector = std::max_element(scoreVector.begin(),scoreVector.end()) ;
                int maxScore = *max_vector ;
                stringScoreMax+=std::to_string(maxScore) ;
                HighScore.setText(stringScoreMax);
                HighScore.LoadTextTexture(font_time,grender) ;
                HighScore.showText(grender,SCREEN_WIDTH-530, 15,0) ;
                HighScore.free() ;
            }

        }
        else if(isHelping)
        {
            while(SDL_PollEvent(&gevent)!= 0)
            {
                if(gevent.type == SDL_QUIT)
                {
                    loadMenu = true ;
                    isHelping = false ;
                }
                else if(gevent.type == SDL_KEYDOWN || gevent.type == SDL_KEYUP)
                {
                    if(gevent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        loadMenu = true ;
                        isHelping = false ;
                    }
                }
            }
            object helpObject ;
            SDL_Texture* helpObjectTexture = nullptr;
            helpObjectTexture = helpObject.LoadTexture("Image//Background//HelpImage.png") ;
            helpObject.draw(grender,helpObjectTexture,nullptr) ;
            helpObject.Free(helpObjectTexture) ;
        }
        else if(isQuitting)
        {
            object quitObject ;
            SDL_Texture* quitObjectTexture = nullptr ;
            quitObjectTexture = quitObject.LoadTexture("Image//Background//QuitImage.png") ;
            quitObject.draw(grender,quitObjectTexture,nullptr) ;

            object buttonYes ;
            SDL_Texture* buttonYesTexture = nullptr ;
            buttonYesTexture = buttonYes.LoadTexture("Image//Background//Yes.png") ;
            buttonYes.setPos(SCREEN_WIDTH/2 - 270,SCREEN_HEIGHT/ 2) ;
            buttonYes.draw(grender,buttonYesTexture,nullptr) ;

            object buttonNo ;
            SDL_Texture* buttonNoTexture = nullptr;
            buttonNoTexture = buttonNo.LoadTexture("Image//Background//No.png") ;
            buttonNo.setPos(SCREEN_WIDTH/2 + 150, SCREEN_HEIGHT/2) ;
            buttonNo.draw(grender,buttonNoTexture,nullptr) ;

            while(SDL_PollEvent(&gevent)!= 0)
            {
                if(gevent.type == SDL_QUIT)
                {
                    isRunning = false ;
                    isQuitting = false ;
                }
                else if(gevent.type == SDL_KEYDOWN || gevent.type == SDL_KEYUP)
                {
                    if(gevent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        loadMenu = true ;
                        isQuitting = false ;
                    }
                }
                else if(gevent.type == SDL_MOUSEBUTTONDOWN || gevent.type == SDL_MOUSEBUTTONUP)
                {
                    if(buttonYes.checkFocus(mousePosx,mousePosy))
                    {
                        isRunning = false ;
                        isQuitting = false ;
                    }
                    else if(buttonNo.checkFocus(mousePosx,mousePosy))
                    {
                        loadMenu = true ;
                        isQuitting = false ;
                    }
                }
            }
            quitObject.Free(quitObjectTexture) ;
            buttonYes.Free(buttonYesTexture) ;
            buttonNo.Free(buttonNoTexture) ;
        }
        else if(endGame)
        {
            object gameOver ;
            SDL_Texture* gameOverTexture = nullptr ;
            gameOverTexture = gameOver.LoadTexture("Image//Background//GameOver.png") ;
            gameOver.draw(grender,gameOverTexture,nullptr) ;
            object buttonYes ;

            SDL_Texture* buttonYesTexture = nullptr;
            buttonYesTexture = buttonYes.LoadTexture("Image//Background//Yes.png") ;
            buttonYes.setPos(SCREEN_WIDTH/2 - 280,SCREEN_HEIGHT/2+30) ;
            buttonYes.draw(grender,buttonYesTexture,nullptr) ;

            object buttonNo ;
            SDL_Texture* buttonNoTexture = nullptr;
            buttonNoTexture = buttonNo.LoadTexture("Image//Background//No.png") ;
            buttonNo.setPos(SCREEN_WIDTH/2 + 130,SCREEN_HEIGHT/2+30) ;
            buttonNo.draw(grender,buttonNoTexture,nullptr) ;

            int randomX = 300 + std::rand()%(1000-350+1) ;
            scoreVector.push_back(scoreValue);
            while(SDL_PollEvent(&gevent)!=0)
            {
                if(gevent.type == SDL_QUIT)
                {
                    loadMenu = true ;
                    endGame  = false ;
                    dino.setState(1) ;
                }
                else if(gevent.type == SDL_KEYUP || gevent.type == SDL_KEYDOWN)
                {
                    if(gevent.key.keysym.sym == SDLK_SPACE)
                    {
                        isStarting = true ;
                        dino.setState(1) ;
                        timeValue = SDL_GetTicks() ;
                        endGame = false ;
                        airplane.setPos(SCREEN_WIDTH + randomX, SCREEN_HEIGHT * 0.7) ;
                        obstacle1.setPos(SCREEN_WIDTH + 2*randomX, SCREEN_HEIGHT - obstacleRect1.h - 10);
                        obstacle2.setPos(SCREEN_WIDTH + 3*randomX, SCREEN_HEIGHT - obstacleRect2.h - 10);
                        obstacle3.setPos(SCREEN_WIDTH + 4*randomX, SCREEN_HEIGHT - obstacleRect3.h - 10);
                        numberPlay++;
                    }
                    else if(gevent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        loadMenu = true ;
                        dino.setState(1) ;
                        endGame = false ;
                        airplane.setPos(SCREEN_WIDTH , SCREEN_HEIGHT * 0.7) ;
                        obstacle1.setPos(SCREEN_WIDTH + 400, SCREEN_HEIGHT - obstacleRect1.h - 10);
                        obstacle2.setPos(SCREEN_WIDTH + 900, SCREEN_HEIGHT - obstacleRect2.h - 10);
                        obstacle3.setPos(SCREEN_WIDTH + 2000, SCREEN_HEIGHT - obstacleRect3.h - 10);
                    }
                }
                else if(gevent.type == SDL_MOUSEBUTTONDOWN || gevent.type == SDL_MOUSEBUTTONUP)
                {
                    if(buttonYes.checkFocus(mousePosx,mousePosy))
                    {
                        Mix_PlayChannel(-1, gClick,0) ;
                        isStarting = true ;
                        dino.setState(1) ;
                        timeValue = SDL_GetTicks() ;
                        endGame = false ;
                        airplane.setPos(SCREEN_WIDTH +5*randomX , SCREEN_HEIGHT * 0.7) ;
                        obstacle1.setPos(SCREEN_WIDTH + 3*randomX, SCREEN_HEIGHT - obstacleRect1.h - 10);
                        obstacle2.setPos(SCREEN_WIDTH + 2*randomX, SCREEN_HEIGHT - obstacleRect2.h - 10);
                        obstacle3.setPos(SCREEN_WIDTH + randomX, SCREEN_HEIGHT - obstacleRect3.h - 10);
                        numberPlay++;

                    }
                    else if(buttonNo.checkFocus(mousePosx,mousePosy))
                    {
                        Mix_PlayChannel(-1,gClick,0);
                        loadMenu = true ;
                        dino.setState(1) ;
                        endGame = false ;
                        numberPlay++ ;
                        airplane.setPos(SCREEN_WIDTH + 1600, SCREEN_HEIGHT * 0.7) ;
                        obstacle1.setPos(SCREEN_WIDTH + 400, SCREEN_HEIGHT - obstacleRect1.h - 10);
                        obstacle2.setPos(SCREEN_WIDTH + 800, SCREEN_HEIGHT - obstacleRect2.h - 10);
                        obstacle3.setPos(SCREEN_WIDTH , SCREEN_HEIGHT - obstacleRect3.h - 10);
                    }

                }
            }
            gameOver.Free(gameOverTexture) ;
            buttonYes.Free(buttonYesTexture) ;
            buttonNo.Free(buttonNoTexture) ;
        }
        SDL_RenderPresent(grender);
    }
    close();
    return 0 ;
}
