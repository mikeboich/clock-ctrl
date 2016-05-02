/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "font.h"

seg_or_flag *system_font[128];

vector_font SpaceChar={
{.flag=0x86},
};
vector_font Exclam={
{01,13,00,22,pos,0x99},
{01,01,02,02,cir,0xff},
{.flag=0x82},

};
vector_font DQuot={
{00,16,00,12,pos,0x99},
{06,16,00,12,pos,0x99},
{.flag=0x86},

};
vector_font Sharp={
{03,12,03,24,pos,0x99},
{8,12,03,24,pos,0x99},
{05,9,15,00,pos,0x99},
{06,15,15,00,pos,0x99},
{.flag=0x8c},

};
vector_font Dollar={
{05,06,10,8,cir,0xf3},
{05,14,10,8,cir,0x3f},
{05,10,00,36,pos,0x99},
{.flag=0x8a},

};
vector_font Percent={
{03,17,06,06,cir,0xff},
{9,03,06,06,cir,0xff},
{06,10,18,30,pos,0x99},
{.flag=0x8c},

};
vector_font Amper={
{04,15,8,10,cir,0xfe},
{04,05,8,10,cir,0x0f},
{04,8,16,16,cir,0xc0},
{06,06,15,18,neg,0x99},
{.flag=0x8c},

};
vector_font Apost={
{03,19,02,02,cir,0xff},
{00,19,8,12,cir,0xc0},
{.flag=0x84},

};
vector_font LParen={
{04,10,8,20,cir,0x0f},
{.flag=0x84},

};
vector_font RParen={
{00,10,8,20,cir,0xf0},
{.flag=0x84},

};
vector_font Aster={
{06,10,18,00,pos,0x99},
{06,10,12,18,pos,0x99},
{06,10,12,18,neg,0x99},
{.flag=0x8c},

};
vector_font Plus={
{06,10,18,00,pos,0x99},
{06,10,00,18,pos,0x99},
{.flag=0x8c},

};
vector_font Comma={
{03,01,02,02,cir,0xff},
{00,01,8,12,cir,0xc0},
{.flag=0x84},

};
vector_font Minus={
{06,10,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font Period={
{01,01,02,02,cir,0xff},
{.flag=0x82},

};
vector_font Slash={
{06,10,18,30,pos,0x99},
{.flag=0x8c},

};
vector_font Colon={
{01,06,02,02,cir,0xff},
{01,14,02,02,cir,0xff},
{.flag=0x82},

};
vector_font SemiCol={
{03,14,02,02,cir,0xff},
{03,06,02,02,cir,0xff},
{00,06,8,12,cir,0xc0},
{.flag=0x84},

};
vector_font LThan={
{06,14,18,12,pos,0x99},
{06,06,18,12,neg,0x99},
{.flag=0x8c},

};
vector_font Equal={
{06,13,18,00,pos,0x99},
{06,07,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font GThan={
{06,14,18,12,neg,0x99},
{06,06,18,12,pos,0x99},
{.flag=0x8c},

};
vector_font Quest={
{05,14,10,10,cir,0xfc},
{05,07,04,04,cir,0xcf},
{05,01,02,02,cir,0xff},
{.flag=0x8a},

};
vector_font AtSign={
{03,10,06,10,cir,0xff},
{03,10,14,20,cir,0xbf},
{8,10,04,04,cir,0xc3},
{.flag=0x8c},

};
vector_font LftSqBr={
{00,10,00,30,pos,0x99},
{02,00,06,00,pos,0x99},
{02,20,06,00,pos,0x99},
{.flag=0x84},

};
vector_font BackSl={
{06,10,18,30,neg,0x99},
{.flag=0x8c},

};
vector_font RtSqBr={
{04,10,00,30,pos,0x99},
{02,00,06,00,pos,0x99},
{02,20,06,00,pos,0x99},
{.flag=0x84},

};
vector_font Carat={
{03,13,9,9,pos,0x99},
{9,13,9,9,neg,0x99},
{.flag=0x8c},

};
vector_font UnderSc={
{06,00,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BackQu={
{02,16,06,12,neg,0x99},
{.flag=0x84},

};
vector_font LfBrace={
{8,06,8,12,cir,0x03},
{00,06,8,8,cir,0x30},
{00,14,8,8,cir,0xc0},
{8,14,8,12,cir,0x0c},
{.flag=0x88},

};
vector_font VertBar={
{01,10,00,36,pos,0x99},
{.flag=0x82},

};
vector_font RtBrace={
{00,06,8,12,cir,0xc0},
{8,06,8,8,cir,0x0c},
{8,14,8,8,cir,0x03},
{00,14,8,12,cir,0x30},
{.flag=0x88},

};
vector_font Tilde={
{03,12,06,04,cir,0x3c},
{9,12,06,04,cir,0xc3},
{.flag=0x8c},

};
vector_font Rubout={
{03,15,9,15,pos,0x99},
{06,10,18,30,pos,0x99},
{9,05,9,15,pos,0x99},
{.flag=0x8c},

};
vector_font Zero={
{06,10,12,20,cir,0xff},
{.flag=0x8c},

};
vector_font One={
{07,10,00,30,pos,0x99},
{05,18,06,06,pos,0x99},
{.flag=0x8c},

};
vector_font Two={
{06,14,12,12,cir,0xfc},
{06,00,12,16,cir,0x0c},
{06,00,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font Three={
{06,06,12,12,cir,0xf1},
{06,20,10,16,cir,0xc0},
{06,20,15,00,pos,0x99},
{.flag=0x8c},

};
vector_font Four={
{8,10,00,30,pos,0x99},
{06,06,18,00,pos,0x99},
{04,13,12,22,pos,0x99},
{.flag=0x8c},

};
vector_font Five={
{06,06,12,12,cir,0xf9},
{03,15,03,15,pos,0x99},
{8,20,12,00,pos,0x99},
{.flag=0x8c},

};
vector_font Six={
{06,06,12,12,cir,0xff},
{05,15,9,15,pos,0x99},
{.flag=0x8c},

};
vector_font Seven={
{06,10,18,30,pos,0x99},
{06,20,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font Eight={
{06,06,12,12,cir,0xff},
{06,16,8,8,cir,0xff},
{.flag=0x8c},

};
vector_font Nine={
{06,14,12,12,cir,0xff},
{07,05,9,15,pos,0x99},
{.flag=0x8c},

};
vector_font BigA={
{03,10,9,30,pos,0x99},
{9,10,9,30,neg,0x99},
{06,8,9,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigB={
{00,10,00,30,pos,0x99},
{04,00,12,00,pos,0x99},
{04,10,12,00,pos,0x99},
{04,20,12,00,pos,0x99},
{8,05,10,10,cir,0xf0},
{8,15,10,10,cir,0xf0},
{.flag=0x8c},

};
vector_font BigC={
{07,10,14,20,cir,0x9f},
{.flag=0x8c},

};
vector_font BigD={
{00,10,00,30,pos,0x99},
{02,00,06,00,pos,0x99},
{02,20,06,00,pos,0x99},
{04,10,16,20,cir,0xf0},
{.flag=0x8c},

};
vector_font BigE={
{00,10,00,30,pos,0x99},
{06,00,18,00,pos,0x99},
{04,10,12,00,pos,0x99},
{06,20,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigF={
{00,10,00,30,pos,0x99},
{04,10,12,00,pos,0x99},
{06,20,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigG={
{07,10,14,20,cir,0x9f},
{11,05,00,9,pos,0x99},
{9,8,06,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigH={
{00,10,00,30,pos,0x99},
{12,10,00,30,pos,0x99},
{06,10,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigI={
{02,10,00,30,pos,0x99},
{02,00,9,00,pos,0x99},
{02,20,9,00,pos,0x99},
{.flag=0x84},

};
vector_font BigJ={
{12,13,00,22,pos,0x99},
{06,06,12,12,cir,0xc3},
{.flag=0x8c},

};
vector_font BigK={
{00,10,00,30,pos,0x99},
{06,05,18,15,neg,0x99},
{06,15,18,15,pos,0x99},
{.flag=0x8c},

};
vector_font BigL={
{00,10,00,30,pos,0x99},
{06,00,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigM={
{00,10,00,30,pos,0x99},
{12,10,00,30,pos,0x99},
{03,15,9,15,neg,0x99},
{9,15,9,15,pos,0x99},
{.flag=0x8c},

};
vector_font BigN={
{00,10,00,30,pos,0x99},
{12,10,00,30,pos,0x99},
{06,10,18,30,neg,0x99},
{.flag=0x8c},

};
vector_font BigO={
{06,10,12,20,cir,0xff},
{.flag=0x8c},

};
vector_font BigP={
{00,10,00,30,pos,0x99},
{04,10,12,00,pos,0x99},
{04,20,12,00,pos,0x99},
{8,15,10,10,cir,0xf0},
{.flag=0x8c},

};
vector_font BigQ={
{06,10,12,20,cir,0xff},
{10,03,06,9,neg,0x99},
{.flag=0x8c},

};
vector_font BigR={
{00,10,00,30,pos,0x99},
{04,10,12,00,pos,0x99},
{04,20,12,00,pos,0x99},
{8,15,10,10,cir,0xf0},
{9,05,9,15,neg,0x99},
{.flag=0x8c},

};
vector_font BigS={
{06,05,12,10,cir,0xf3},
{06,15,12,10,cir,0x3f},
{.flag=0x8c},

};
vector_font BigT={
{06,10,00,30,pos,0x99},
{06,20,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font BigU={
{00,13,00,22,pos,0x99},
{12,13,00,22,pos,0x99},
{06,06,12,12,cir,0xc3},
{.flag=0x8c},

};
vector_font BigV={
{03,10,9,30,neg,0x99},
{9,10,9,30,pos,0x99},
{.flag=0x8c},

};
vector_font BigW={
{00,10,00,30,pos,0x99},
{12,10,00,30,pos,0x99},
{03,05,9,15,pos,0x99},
{9,05,9,15,neg,0x99},
{.flag=0x8c},

};
vector_font BigX={
{06,10,18,30,neg,0x99},
{06,10,18,30,pos,0x99},
{.flag=0x8c},

};
vector_font BigY={
{06,05,00,15,pos,0x99},
{03,15,9,15,neg,0x99},
{9,15,9,15,pos,0x99},
{.flag=0x8c},

};
vector_font BigZ={
{06,10,18,30,pos,0x99},
{06,00,18,00,pos,0x99},
{06,20,18,00,pos,0x99},
{.flag=0x8c},

};
vector_font SmallA={
{05,06,10,12,cir,0xff},
{10,06,00,18,pos,0x99},
{.flag=0x8a},

};
vector_font SmallB={
{05,06,10,12,cir,0xff},
{00,10,00,30,pos,0x99},
{.flag=0x8a},

};
vector_font SmallC={
{05,06,10,12,cir,0x9f},
{.flag=0x88},

};
vector_font SmallD={
{05,06,10,12,cir,0xff},
{10,10,00,30,pos,0x99},
{.flag=0x8a},

};
vector_font SmallE={
{05,06,10,12,cir,0xbf},
{05,06,15,00,pos,0x99},
{.flag=0x8a},

};
vector_font SmallF={
{07,16,06,8,cir,0x3c},
{04,10,12,00,pos,0x99},
{04,8,00,24,pos,0x99},
{.flag=0x8a},

};
vector_font SmallG={
{05,06,10,12,cir,0xff},
{10,06,00,18,pos,0x99},
{05,00,10,12,cir,0xc1},
{.flag=0x8a},

};
vector_font SmallH={
{04,8,8,8,cir,0x3c},
{00,10,00,30,pos,0x99},
{8,04,00,12,pos,0x99},
{.flag=0x88},

};
vector_font SmallI={
{01,16,02,02,cir,0xff},
{01,06,00,18,pos,0x99},
{.flag=0x82},

};
vector_font SmallJ={
{06,16,02,02,cir,0xff},
{06,06,00,18,pos,0x99},
{03,00,06,8,cir,0xc1},
{.flag=0x88},

};
vector_font SmallK={
{00,10,00,30,pos,0x99},
{04,8,12,12,pos,0x99},
{04,03,9,9,neg,0x99},
{.flag=0x88},

};
vector_font SmallL={
{01,10,00,30,pos,0x99},
{.flag=0x82},

};
vector_font SmallM={
{00,06,00,18,pos,0x99},
{04,8,8,8,cir,0x3c},
{8,04,00,12,pos,0x99},
{12,8,8,8,cir,0x3c},
{16,04,00,12,pos,0x99},
{.flag=0x90},

};
vector_font SmallN={
{00,06,00,18,pos,0x99},
{04,8,8,8,cir,0x3c},
{8,04,00,12,pos,0x99},
{.flag=0x88},

};
vector_font SmallO={
{05,06,10,12,cir,0xff},
{.flag=0x8a},

};
vector_font SmallP={
{05,06,10,12,cir,0xff},
{00,03,00,24,pos,0x99},
{.flag=0x8a},

};
vector_font SmallQ={
{05,06,10,12,cir,0xff},
{10,03,00,24,pos,0x99},
{.flag=0x8a},

};
vector_font SmallR={
{00,06,00,18,pos,0x99},
{05,06,10,12,cir,0x1c},
{.flag=0x88},

};
vector_font SmallS={
{04,9,8,06,cir,0x3f},
{04,03,8,06,cir,0xf3},
{.flag=0x88},

};
vector_font SmallT={
{8,04,8,8,cir,0x03},
{04,12,12,00,pos,0x99},
{04,10,00,18,pos,0x99},
{.flag=0x88},

};
vector_font SmallU={
{8,06,00,18,pos,0x99},
{04,04,8,8,cir,0xc3},
{00,8,00,12,pos,0x99},
{.flag=0x88},

};
vector_font SmallV={
{02,06,06,18,neg,0x99},
{06,06,06,18,pos,0x99},
{.flag=0x88},

};
vector_font SmallW={
{02,06,06,18,neg,0x99},
{06,06,06,18,pos,0x99},
{10,06,06,18,neg,0x99},
{14,06,06,18,pos,0x99},
{.flag=0x90},

};
vector_font SmallX={
{04,06,12,18,neg,0x99},
{04,06,12,18,pos,0x99},
{.flag=0x88},

};
vector_font SmallY={
{02,06,06,18,neg,0x99},
{06,06,06,18,pos,0x99},
{01,00,06,8,cir,0xc0},
{.flag=0x88},

};
vector_font SmallZ={
{04,00,12,00,pos,0x99},
{04,12,12,00,pos,0x99},
{04,06,12,18,pos,0x99},
{.flag=0x88},

};
vector_font JDay={
{02,19,06,00,pos,0x99},
{02,11,06,00,pos,0x99},
{02,04,06,00,pos,0x99},
{00,11,00,23,pos,0x99},
{04,11,00,22,pos,0x99},
{9,22,9,00,pos,0x99},
{9,18,9,00,pos,0x99},
{9,14,9,00,pos,0x99},
{12,18,00,12,pos,0x99},
{17,22,9,00,pos,0x99},
{17,18,9,00,pos,0x99},
{17,14,9,00,pos,0x99},
{20,18,00,12,pos,0x99},
{15,10,17,00,pos,0x99},
{14,07,17,00,pos,0x99},
{14,04,17,00,pos,0x99},
{15,01,20,00,pos,0x99},
{8,04,00,14,pos,0x99},
{14,05,00,14,pos,0x99},
{00,12,19,14,cir,0x40},
{15,11,02,03,pos,0x81},
{.flag=0x98},
};
vector_font JSun={
{8,19,18,00,pos,0x99},
{8,10,18,00,pos,0x99},
{8,18,00,00,pos,0x99},
{02,10,00,26,pos,0x99},
{14,10,00,26,pos,0x99},
{.flag=0x93},
};
vector_font JMon={
{9,20,14,00,pos,0x99},
{9,13,14,00,pos,0x99},
{9,06,14,00,pos,0x99},
{04,13,00,21,pos,0x99},
{00,06,8,12,cir,0xC0},
{14,10,00,30,pos,0x99},
{13,00,03,00,pos,0x81},
{.flag=0x91},
};
vector_font JTue={
{10,15,00,17,pos,0x99},
{00,10,20,20,cir,0xC0},
{20,10,20,20,cir,0x03},
{00,16,8,15,cir,0x40},
{15,14,03,07,pos,0x99},
{.flag=0x93},
};
vector_font JWed={
{10,11,00,33,pos,0x99},
{9,00,03,00,pos,0x81},
{04,14,07,00,pos,0x99},
{00,14,14,19,cir,0x40},
{03,05,05,07,pos,0x99},
{15,13,8,06,pos,0x99},
{27,19,34,33,cir,0x02},
{17,05,07,07,neg,0x99},
{.flag=0x94},
};
vector_font JThu={
{10,11,00,34,pos,0x99},
{10,15,26,00,pos,0x99},
{01,15,18,22,cir,0x40},
{04,04,10,10,pos,0x99},
{19,15,18,22,cir,0x02},
{16,04,10,10,neg,0x99},
{.flag=0x93},
};
vector_font JFri={
{00,22,21,17,cir,0x40},
{04,14,9,06,pos,0x99},
{21,22,22,17,cir,0x02},
{16,14,8,06,neg,0x99},
{10,14,12,00,pos,0x99},
{10,9,25,00,pos,0x99},
{10,07,00,19,pos,0x99},
{10,01,26,00,pos,0x99},
{03,03,8,10,cir,0x20},
{11,06,07,10,cir,0x40},
{.flag=0x94},
};
vector_font JSat={
{10,12,00,31,pos,0x99},
{10,14,23,00,pos,0x99},
{10,02,28,00,pos,0x99},
{.flag=0x95},
};
vector_font JYear={
{00,21,10,12,cir,0x40},
{02,16,04,04,pos,0x99},
{10,18,18,00,pos,0x99},
{10,12,16,00,pos,0x99},
{9,06,26,00,pos,0x99},
{05,9,00,9,pos,0x99},
{10,8,00,29,pos,0x99},
{.flag=0x92},
};
vector_font JTEST={
{28,19,37,32,cir,0x02},
{.flag=0x92}}; 

vector_font TestBox={
{0,0, 255,0,pos,0x99},
{0,0, 0,255 ,pos,0x99},
{.flag=0x84},

};

vector_font TestCircle={
{127,127, 2,2,cir,0xff},
{.flag=0x84},

};

vector_font Test_Pat = {
{0,0, 255,0,pos,0x99},
{0,0, 0,255 ,pos,0x99},
{0,0, 255,255,cir,0xff},
{.flag=0x84}
};
void init_font(){
    system_font[0] = (seg_or_flag*)&SpaceChar;
    system_font[1] = (seg_or_flag*)&Exclam;
    system_font[2] = (seg_or_flag*)&DQuot;
    system_font[3] = (seg_or_flag*)&Sharp;
    system_font[4] = (seg_or_flag*)&Dollar;
    system_font[5] = (seg_or_flag*)&Percent;
    system_font[6] = (seg_or_flag*)&Amper;
    system_font[7] = (seg_or_flag*)&Apost;
    system_font[8] = (seg_or_flag*)&LParen;
    system_font[9] = (seg_or_flag*)&RParen;
    system_font[10] = (seg_or_flag*)&Aster;
    system_font[11] = (seg_or_flag*)&Plus;
    system_font[12] = (seg_or_flag*)&Comma;
    system_font[13] = (seg_or_flag*)&Minus;
    system_font[14] = (seg_or_flag*)&Period;
    system_font[15] = (seg_or_flag*)&Slash;
    system_font[16] = (seg_or_flag*)&Zero;
    system_font[17] = (seg_or_flag*)&One;
    system_font[18] = (seg_or_flag*)&Two;
    system_font[19] = (seg_or_flag*)&Three;
    system_font[20] = (seg_or_flag*)&Four;
    system_font[21] = (seg_or_flag*)&Five;
    system_font[22] = (seg_or_flag*)&Six;
    system_font[23] = (seg_or_flag*)&Seven;
    system_font[24] = (seg_or_flag*)&Eight;
    system_font[25] = (seg_or_flag*)&Nine;
    system_font[26] = (seg_or_flag*)&Colon;
    system_font[27] = (seg_or_flag*)&SemiCol;
    system_font[28] = (seg_or_flag*)&LThan;
    system_font[29] = (seg_or_flag*)&Equal;
    system_font[30] = (seg_or_flag*)&GThan;
    system_font[31] = (seg_or_flag*)&Quest;
    system_font[32] = (seg_or_flag*)&AtSign;
    system_font[33] = (seg_or_flag*)&BigA;
    system_font[34] = (seg_or_flag*)&BigB;
    system_font[35] = (seg_or_flag*)&BigC;
    system_font[36] = (seg_or_flag*)&BigD;
    system_font[37] = (seg_or_flag*)&BigE;
    system_font[38] = (seg_or_flag*)&BigF;
    system_font[39] = (seg_or_flag*)&BigG;
    system_font[40] = (seg_or_flag*)&BigH;
    system_font[41] = (seg_or_flag*)&BigI;
    system_font[42] = (seg_or_flag*)&BigJ;
    system_font[43] = (seg_or_flag*)&BigK;
    system_font[44] = (seg_or_flag*)&BigL;
    system_font[45] = (seg_or_flag*)&BigM;
    system_font[46] = (seg_or_flag*)&BigN;
    system_font[47] = (seg_or_flag*)&BigO;
    system_font[48] = (seg_or_flag*)&BigP;
    system_font[49] = (seg_or_flag*)&BigQ;
    system_font[50] = (seg_or_flag*)&BigR;
    system_font[51] = (seg_or_flag*)&BigS;
    system_font[52] = (seg_or_flag*)&BigT;
    system_font[53] = (seg_or_flag*)&BigU;
    system_font[54] = (seg_or_flag*)&BigV;
    system_font[55] = (seg_or_flag*)&BigW;
    system_font[56] = (seg_or_flag*)&BigX;
    system_font[57] = (seg_or_flag*)&BigY;
    system_font[58] = (seg_or_flag*)&BigZ;
    system_font[59] = (seg_or_flag*)&LftSqBr;
    system_font[60] = (seg_or_flag*)&BackSl;
    system_font[61] = (seg_or_flag*)&RtSqBr;
    system_font[62] = (seg_or_flag*)&Carat;
    system_font[63] = (seg_or_flag*)&UnderSc;
    system_font[64] = (seg_or_flag*)&BackQu;
    system_font[65] = (seg_or_flag*)&SmallA;
    system_font[66] = (seg_or_flag*)&SmallB;
    system_font[67] = (seg_or_flag*)&SmallC;
    system_font[68] = (seg_or_flag*)&SmallD;
    system_font[69] = (seg_or_flag*)&SmallE;
    system_font[70] = (seg_or_flag*)&SmallF;
    system_font[71] = (seg_or_flag*)&SmallG;
    system_font[72] = (seg_or_flag*)&SmallH;
    system_font[73] = (seg_or_flag*)&SmallI;
    system_font[74] = (seg_or_flag*)&SmallJ;
    system_font[75] = (seg_or_flag*)&SmallK;
    system_font[76] = (seg_or_flag*)&SmallL;
    system_font[77] = (seg_or_flag*)&SmallM;
    system_font[78] = (seg_or_flag*)&SmallN;
    system_font[79] = (seg_or_flag*)&SmallO;
    system_font[80] = (seg_or_flag*)&SmallP;
    system_font[81] = (seg_or_flag*)&SmallQ;
    system_font[82] = (seg_or_flag*)&SmallR;
    system_font[83] = (seg_or_flag*)&SmallS;
    system_font[84] = (seg_or_flag*)&SmallT;
    system_font[85] = (seg_or_flag*)&SmallU;
    system_font[86] = (seg_or_flag*)&SmallV;
    system_font[87] = (seg_or_flag*)&SmallW;
    system_font[88] = (seg_or_flag*)&SmallX;
    system_font[89] = (seg_or_flag*)&SmallY;
    system_font[90] = (seg_or_flag*)&SmallZ;
    system_font[91] = (seg_or_flag*)&LfBrace;
    system_font[92] = (seg_or_flag*)&VertBar;
    system_font[93] = (seg_or_flag*)&RtBrace;
    system_font[94] = (seg_or_flag*)&Tilde;
    system_font[95] = (seg_or_flag*)&Rubout;
    system_font[96] = (seg_or_flag*)&JDay;
    system_font[97] = (seg_or_flag*)&JSun;
    system_font[98] = (seg_or_flag*)&JMon;
    system_font[99] = (seg_or_flag*)&JTue;
    system_font[100] = (seg_or_flag*)&JWed;
    system_font[101] = (seg_or_flag*)&JThu;
    system_font[102] = (seg_or_flag*)&JFri;
    system_font[103] = (seg_or_flag*)&JSat;
    system_font[104] = (seg_or_flag*)&JYear;
    system_font[105] = (seg_or_flag*)&JTEST;
}

// pins integer values to uint8 range, rather than letting them wrap around:
uint8 pin(int x){
  if(x<0)x=0;
  if(x>255)x=255;
  return x;
}

// returns the width of a single vector character:
int char_width(char c){
  seg_or_flag *seg_ptr = system_font[((uint8) c)-32];    // map from char code to segment list
  while(seg_ptr->seg_data.x_offset<0x80) seg_ptr++;       // skip over segments
  return seg_ptr->flag & 0x7f;                 // end flag - 0x80 + char width
}

// returns the width (sum of character widths) of a string:
uint8 stringWidth(char s[],uint8 scale){
  int width=0,index=0;
    
  while(*s){
    width += char_width(*s);
    s++;
  }
  return pin(width*scale); 
}
