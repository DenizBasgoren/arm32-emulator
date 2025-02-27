#define SRC 0x40000030
#define TARGET 0x40000038
#define TXT 0x40000024

#define gpuClear(mod) *(char*)(0x40000010) = mod
#define gpuDraw(mod) *(char*)(0x40000011) = mod
#define gpuUpdate() *(char*)(0x40000012) = 1


#define i8 char
#define i16 short
#define i32 int
#define i64 long long int

const char* data = "~!!~!!~!!~~!~~!~~!~~!~~!\
~!!~!!~!!~~!~~!~~!~~!~~!\
~!!~!!~!!~~!~~!~~!~~!~~!\
!!~!!~!!~!~!!~!!~!!~!!~!\
!!~!!~!!~!~!!~!!~!!~!!~!\
!!~!!~!!~!~!!~!!~!!~!!~!";

int slot = 0;

void setColor(i32 color)
{
    i32* c = (void*)(0x40000020);
    *c = color;
}

void setSrc(i16 x, i16 y, i16 w, i16 h)
{
    i16* pos = (void*)SRC;
    pos[0] = x;
    pos[1] = y;
    pos[2] = w;
    pos[3] = h;
}

void setTarget(i16 x, i16 y, i16 w, i16 h)
{
    i16* pos = (void*)TARGET;
    pos[0] = x;
    pos[1] = y;
    pos[2] = w;
    pos[3] = h;
}

void loadTexture(const char* data, int w, int h)
{
    i16* wh = (void*)TXT;
    wh[0] = w;
    wh[1] = h;

    i32* addr = (void*)(TXT + 4);
    *addr = (void*)data;
    i8* chn = (void*)(TXT + 8);
    *chn = 3;
    
    gpuUpdate();
}

void _start()
{
    loadTexture(data, 8, 6);

    setSrc(0, 0, 1, 6);
    setTarget(0, 0, 50, 600);

    int pos_x = 0;

    while(1)
    {
        gpuClear(0);

        setSrc(pos_x>>6, 0, 1, 6);
        setTarget(pos_x, 0, 50, 600);

        gpuDraw(3);

        for(int wait = 0; wait < 0x20000; wait++);

        pos_x = (pos_x+1)%512;
    }
}