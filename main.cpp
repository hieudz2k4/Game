#include <bits/stdc++.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

//KHAI BAO CAC THONG SO CO DINH
const int SCREEN_HEIGHT = 600 ;
const int SCREEN_WIDTH = 1200;
const int FRAME = 60 ;
const int DINO_WIDTH = 64;
const int DINO_HEIGHT = 64;
const int COLOR_KEY_R = 167;
const int COLOR_KEY_G = 175 ;
const int COLOR_KEY_B = 180 ;
const int RENDER_DRAW_COLOR = 255 ;
const int FPS = 60 ;
const int MAX_OBSTACLES = 3 ;

SDL_Window* gwindow  = nullptr ;
SDL_Renderer* grender = nullptr ;
SDL_Texture* gDinoTexture = nullptr ;
SDL_Texture* gBackgroundTexture = nullptr ;
SDL_Texture* gObstacleTexture = nullptr ;
SDL_Event gevent ;

//âm thanh
Mix_Music* gMenuSound = nullptr ;
Mix_Chunk* gJump = nullptr ;
Mix_Chunk* gDead = nullptr ;
Mix_Chunk* gPoint = nullptr ;
bool isRunning = true ;
bool endGame = false;
bool loadMenu = true ;

//text
TTF_Font* font_time = nullptr;

class object
{
protected :
    SDL_Rect rect_ ;
public:
    int x_val, y_val ;
    object()
    {
        rect_.x = 0 ;
        rect_.y = 0 ;
        rect_.w = 0 ;
        rect_.h = 0 ;
        x_val = 0 ;
        y_val = 0 ;
    };
    ~object(){};
    void setPos(const int& x, const int& y){rect_.x = x ; rect_.y = y ;}
    SDL_Rect getRect() const{return rect_ ;}
    SDL_Texture* LoadTexture(std::string path)
    {
        SDL_Texture* new_texture = nullptr ;

    //DOC TAM ANH THANH SURFACE
        SDL_Surface* loadsurface = IMG_Load(path.c_str()) ;
        if(loadsurface != NULL)
        {
        //TRANPARENCE BACKGROUND
        SDL_SetColorKey(loadsurface, SDL_TRUE,SDL_MapRGB(loadsurface->format ,COLOR_KEY_R,COLOR_KEY_G,COLOR_KEY_B)) ;

        //CHUYEN SURFACE THANH TEXTURE LUU DU TAT CA THONG TIN VE TAM ANH
        new_texture = SDL_CreateTextureFromSurface(grender, loadsurface) ;
        if(new_texture != nullptr)
        {
            rect_.w = loadsurface->w ;
            rect_.h = loadsurface->h ;
        }

        //FREE(XOA) SURFACE
        SDL_FreeSurface(loadsurface) ;
        }
        return new_texture;
        SDL_DestroyTexture(new_texture) ;

    }
    void draw(SDL_Renderer* render,SDL_Texture* texture,const SDL_Rect* srcRect)
    {
        SDL_Rect renderquad = {rect_.x,rect_.y,rect_.w,rect_.h} ;
        //DAY DOI TUONG LEN MAN HINH
        SDL_RenderCopy(render,texture,srcRect,&renderquad) ;
    }
    void Free(SDL_Texture* texture)
    {
        SDL_DestroyTexture(texture) ;
    }
};

class Dino:public object
{
private :
    enum DinoState
    {
        DEAD = 0 ,
        RUN = 1 ,
        JUMP = 2 ,
        DOWN = 3
    };
    float gravity, jumpVel ;
    int maxHeight,minHeight,x_pos,y_pos;
    int state ;
    bool onGround ;
public:
    Dino()
    {
        gravity = 20 ;
        jumpVel = -DINO_HEIGHT;
        maxHeight = SCREEN_HEIGHT - DINO_HEIGHT - 200 ;
        minHeight = SCREEN_HEIGHT - DINO_HEIGHT - 10 ;
        x_pos = 10 ;
        y_pos = minHeight ;
        state = 1 ;
        onGround = true ;

    }
    ~Dino(){} ;
    void Show()
    {
        //set lại vị trí hiển thị dino
        rect_.x = x_pos ;
        rect_.y = y_pos ;
        switch(state)
        {
        case DOWN :
            gDinoTexture = LoadTexture("Image//Dino//Dino_Down.png") ;
            draw(grender, gDinoTexture,NULL);
            break ;
        case DEAD:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Dead.png");
            draw(grender, gDinoTexture,NULL);
            Mix_PlayChannel(-1,gDead,0) ;
            break;
        case JUMP :
            gDinoTexture = LoadTexture("Image//Dino//Dino_Jump.png");
            if(onGround == true)
            {
                rect_.y -= 200 ;
                draw(grender,gDinoTexture,NULL) ;
                Mix_PlayChannel(-1,gJump,0) ;
            }


            break ;
        case RUN:
            gDinoTexture = LoadTexture("Image//Dino//Dino_Run.png") ;
            rect_.w = DINO_WIDTH ;
            rect_.h = DINO_HEIGHT ;
            int frameNum = 2 ;
            int frameDuration = 100 ;
            SDL_Rect Frame[frameNum] ;
            Frame[0] = {0,0,DINO_WIDTH,DINO_HEIGHT} ;
            Frame[1] = {DINO_WIDTH,0,DINO_WIDTH,DINO_HEIGHT} ;
            int currentframe = (SDL_GetTicks()/frameDuration)%frameNum  ;
            draw(grender, gDinoTexture,&Frame[currentframe]);
            break ;
        }
        Free(gDinoTexture) ;
    }
    void HandleEvent(SDL_Event event)
    {
        if(event.type==SDL_KEYDOWN)
        {
            if(event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym ==SDLK_UP)
                state = JUMP ;
            else if(event.key.keysym.sym ==SDLK_DOWN)
                state = DOWN ;
        }
        else if (event.type==SDL_KEYUP)
        {
            if(event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym ==SDLK_UP)
            {
                state = JUMP ;
                state = RUN ;
            }
            else if(event.key.keysym.sym ==SDLK_DOWN)
            {
                state = DOWN ;
                state = RUN ;
            }
        }

    }
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

        if (dinoRight > obstacleLeft && dinoLeft < obstacleRight &&
        dinoBottom > obstacleTop && dinoTop < obstacleBottom)
        {
            state = DEAD ;
            return true; // Có va chạm
        }
        if (dinoRight > obstacleLeft && dinoLeft < obstacleRight &&
        dinoBottom > obstacleTop && dinoTop < obstacleBottom)
        {
            state = DEAD ;
            return true;
        }

        return false ;
    }
    bool DinoOnGround()
    {
        if(rect_.y = minHeight)
            onGround = true ;
        else
            onGround = false ;
        return onGround ;
    }
};

class Obstacle: public object
{
private:

public:
    Obstacle()
    {
        rect_.x =0;
        rect_.y =0;
        rect_.w = 0 ;
        rect_.h = 0 ;
        x_val = -1 ;
        y_val = 0 ;
    }
    ~Obstacle(){;}
    void HandleMove(const int& x_border, const int& y_border)
    {
        rect_.x += x_val ;
        if(rect_.x < 0 )
        {
            rect_.x = SCREEN_WIDTH ;
        }
    }
    void HandleInputAction(SDL_Event event);
    void setXVal(const int& newXVal){x_val = newXVal ; }

};

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
    void free()
    {
        SDL_DestroyTexture(textTexture) ;
        textTexture = nullptr ;
    }
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
    void setText(const std::string& text){stringText= text ; }
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
/*
void showMenu()
{
    object Menu;
    SDL_Texture* gMenuTexture = nullptr;
    gMenuTexture = Menu.LoadTexture("Image//Background//Menu.png") ;
    Menu.draw(gMenuTexture) ;
}
*/
bool Init()
{
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

    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT,2,4096) == -1)
    {
        std::cout << "Lỗi âm thanh" ;
        success = false ;
    }
    gDead = Mix_LoadWAV("Sound//Dead.wav") ;
    gJump = Mix_LoadWAV("Sound//Jump.wav") ;
    gMenuSound = Mix_LoadMUS("Sound//Clown.wav") ;
    if(gDead == NULL || gJump == NULL || gMenuSound == NULL)
    {
        std::cout << "loi load file am thanh" ;
        success = false ;
    }


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
    SDL_DestroyTexture(gObstacleTexture) ;
    gObstacleTexture = nullptr ;
    Mix_FreeChunk(gJump) ;
    gJump = nullptr ;
    Mix_FreeChunk(gDead) ;
    gDead = nullptr ;
    Mix_FreeMusic(gMenuSound);
    gMenuSound = nullptr ;
    IMG_Quit() ;
    SDL_QUIT;
}
int main(int argc, char* argv[])
{
    if(Init()==false)
        std::cout << "Khoi tao khong thanh cong" ;
    //Background
    object Background ;
    gBackgroundTexture = Background.LoadTexture("Image//Background//city1.png") ;
    //Dino
    Dino dino ;

    //Obstacles
    Obstacle obstacle ;
    //sinh ra path file ngẫu nhiên
    /*
    std::string directory = "Image//Obstacles//" ;
    std::string fileExtension = ".png" ;
    std::srand(std::time(0));
    int i = std::rand()%(16)+1 ;
    std::string fileName = "Ob"+std::to_string(i) ;
    std::string filePath = directory+fileName+fileExtension ;
    std::cout << filePath ;
    gObstacleTexture = obstacle.LoadTexture(filePath) ;
    SDL_Rect obstacleRect = obstacle.getRect() ;
    int obstaclex = SCREEN_WIDTH - obstacleRect.w ;
    int obstacley = SCREEN_HEIGHT-obstacleRect.h - 5 ;
    obstacle.setPos(obstaclex,obstacley)  ;
    */


    //Obstacle airplane
    Obstacle airplane ;
    SDL_Texture* airplaneTexture = nullptr ;
    airplaneTexture = airplane.LoadTexture("Image//Obstacles//Ob17.png");

    Text score_game ;
    score_game.setColor(Text::YELLOW);
    Text gameOver ;
    gameOver.setColor(Text::ColorText::WHITE) ;

    while(isRunning)
    {
        while(SDL_PollEvent(&gevent)!=0)
        {
            if(gevent.type == SDL_QUIT)
            {
                isRunning = false ;
            }
            dino.HandleEvent(gevent) ;
        }


        SDL_SetRenderDrawColor(grender, RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR,RENDER_DRAW_COLOR) ;
        SDL_RenderClear(grender) ;

        Uint32 timeValue = SDL_GetTicks() / 1000;
        //Menu
        if(loadMenu == true)
        {
            object Menu;
            SDL_Texture* gMenu ;
            gMenu = Menu.LoadTexture("Image//Background//Menu1.png");
            Menu.draw(grender,gMenu,nullptr) ;
            Menu.Free(gMenu) ;

            object buttonPlay ;
            SDL_Texture* buttonPlayTexture ;
            buttonPlayTexture = buttonPlay.LoadTexture("Image//Background//Play1.png") ;
            buttonPlay.setPos(520,SCREEN_HEIGHT/2) ;
            buttonPlay.draw(grender,buttonPlayTexture,nullptr);
            buttonPlay.Free(buttonPlayTexture) ;

            object buttonHelp;
            SDL_Texture* buttonHelpTexture ;
            buttonHelpTexture = buttonHelp.LoadTexture("Image//Background//Help1.png") ;
            buttonHelp.setPos(520,SCREEN_HEIGHT/2 + 100) ;
            buttonHelp.draw(grender,buttonHelpTexture,nullptr);
            buttonHelp.Free(buttonHelpTexture) ;

            object buttonExit ;
            SDL_Texture* buttonExitTexture ;
            buttonExitTexture = buttonExit.LoadTexture("Image//Background//Quit1.png") ;
            buttonExit.setPos(520,SCREEN_HEIGHT/2+200) ;
            buttonExit.draw(grender,buttonExitTexture,nullptr);
            buttonExit.Free(buttonExitTexture) ;

            Mix_PlayMusic(gMenuSound,-1) ;

        }
        else{
        //BackGround
        if(timeValue<= 15)
            Background.x_val -= 1 ;
        else if(timeValue>=15)
            Background.x_val -= 2 ;
        else if(timeValue>=35)
            Background.x_val -= 3 ;
        else if(timeValue>= 45)
            Background.x_val -= 4 ;
        else if(timeValue>= 55)
            Background.x_val -= 5 ;

        Background.setPos(Background.x_val, 0);
        Background.draw(grender , gBackgroundTexture,nullptr) ;
        Background.setPos(Background.x_val+SCREEN_WIDTH,0) ;
        Background.draw(grender, gBackgroundTexture,nullptr) ;
        if(Background.x_val <= -SCREEN_WIDTH)
        {
            Background.x_val=  0 ;
        }

        //Dino
        dino.Show() ;
        dino.checkCollision(obstacle.getRect());

        //airplane
        airplane.draw(grender, airplaneTexture,nullptr) ;
        airplane.HandleMove(SCREEN_WIDTH, SCREEN_HEIGHT) ;
        airplane.Free(airplaneTexture) ;

        //Obstacles
        for(int i = 1 ; i<=MAX_OBSTACLES;i++)
        {
            std::string directory = "Image//Obstacles//" ;
            std::string fileExtension = ".png" ;
            std::srand(std::time(0));
            int j = std::rand()%(16)+1 ;
            std::string fileName = "Ob"+std::to_string(j) ;
            std::string filePath = directory+fileName+fileExtension ;
            //std::cout << filePath ;
            gObstacleTexture = obstacle.LoadTexture(filePath) ;
            SDL_Rect obstacleRect = obstacle.getRect() ;
            //int xRandom = std::rand()%(200+1) ;
            int obstaclex = SCREEN_WIDTH - obstacleRect.w ;
            int obstacley = SCREEN_HEIGHT-obstacleRect.h - 5 ;
            obstacle.setPos(obstaclex,obstacley)  ;
            obstacle.draw(grender,gObstacleTexture,nullptr) ;
            obstacle.HandleMove(SCREEN_WIDTH,SCREEN_HEIGHT) ;
        }

        std::string stringScore = "Score: " ;
        Uint32 scoreValue = 10*SDL_GetTicks() / 1000;
        std::string scoreToString = std::to_string(scoreValue);
        stringScore+=scoreToString ;
        score_game.setText(stringScore);
        score_game.LoadTextTexture(font_time,grender) ;
        score_game.showText(grender,SCREEN_WIDTH- 220,15,0) ;
        score_game.free() ;
        }

        SDL_RenderPresent(grender);
    }
    close();
    return 0 ;
}
