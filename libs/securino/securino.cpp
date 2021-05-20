#include "Arduino.h"
#include "securino.h"

void Securino::show(String a, byte vet[]){
  Serial.print(a);  
  for(int i=0; i < 16; i++)
  {
      Serial.print(vet[i], HEX);
      Serial.print(" ");  
  }
  Serial.println();  
}

void Securino::show(String a, String b, byte vet[]){
  Serial.print(a);  
  for(int i=0; i < 32; i++)
  {
      Serial.print(vet[i], HEX);
      Serial.print(" ");  
  }
  Serial.println();  
}

//-------------- Variaveis Globais------------------------------
byte plaintext[4][4];
byte key[4][4];                 
byte keyCopia[4][4];


//--------------Funcoes Públicas -------------------------------
byte * Securino::encript(String operacao, byte in[], byte k[] ){
  setKey(k);
  
  if(operacao=="aes-128-ecb"){
    
    setData(in);
    encAES128(plaintext,key);
    
    return getData();
    
  }
  if(operacao=="aes-128-cbc"){
    
    byte IV[16];
   
    byte auxin[16];
    geraVetorInicializacao(IV);
     show("IV: ",IV);
    // continuar daqui...
    for(byte i=0x00; i<0x10; i++){
      auxin[i] = in[i] ^ IV[i];
    }
    
    
    setData(auxin);
    //setData(in);
    encAES128(plaintext, key);
    
    return getDataCBC(IV);
    
  }
  
}

byte * Securino::decript(String operacao, byte in[], byte k[] ){
  setKey(k);
  
  if(operacao=="aes-128-ecb"){
    setData(in);
    return getData();
    decAES128(plaintext,key);
  }else if(operacao=="aes-128-cbc"){
    byte state[16];
    byte auxIV[16];

    split(state,auxIV, in);
    
    setData(state);
    
    decAES128(plaintext, key);
     int veti = 0;
  //De vetor[16] para matris[4][4]
  for(int j=0; j < 4; j++){
    for(int i=0; i < 4; i++){
      plaintext[i][j] = plaintext[i][j] ^ auxIV[veti];
      veti++;
    }
  }
   
   return getData();
  }   
}

//--------------Funcoes Privadas -------------------------------
void Securino::setKey(byte x[16]){
  int veti = 0;
  //De vetor[16] para matris[4][4]
  for(int j=0; j < 4; j++){
    for(int i=0; i < 4; i++){
      key[i][j] = x[veti];
      keyCopia[i][j] = x[veti];
      veti++;
    }
  }
}

void Securino::setData(byte x[16]){
  int veti = 0;
  //De vetor[16] para matris[4][4]
  for(int j=0; j < 4; j++){
    for(int i=0; i < 4; i++){
      plaintext[i][j] = x[veti];
      veti++;
    }
  }
}

byte * Securino::getData(){
  static byte xxy[16];
  int veti = 0;
  //De  matris[4][4] para vetor[16]
  for(int j=0; j < 4; j++){
    for(int i=0; i < 4; i++){
      xxy[veti] = plaintext[i][j];
      veti++;
    }
  }
  return xxy;
}

byte * Securino::getDataCBC(byte randKey[]){
  
  static byte xyy[32];
  int veti=0;
    for(byte i=0x00; i<0x10; i++){
      xyy[i] = randKey[i];
    }
    
    for(int j=0; j < 4; j++){
    for(int i=0; i < 4; i++){
      xyy[veti+16] = plaintext[i][j];
      veti++;
    }
  }
  
  return xyy;

}

void Securino::encAES128(byte plainText[][4], byte chave[][4]){
  AddRoundKey(plainText, chave);
  for(int i=1; i<10; i++){
    subBytes(plainText);
    ShiftRows(plainText);
    MixColumns(plainText);
    KeySchedule(chave, i);
    AddRoundKey(plainText, chave);
  }
  subBytes(plainText);
  ShiftRows(plainText);
  KeySchedule(chave, 10);
  AddRoundKey(plainText, chave);
}

void Securino::decAES128(byte plainText[][4], byte chave[][4]){
  InvKeySchedule(chave, 10);
  AddRoundKey(plainText, chave);
  for(int i=9; i>=1; i--){  
    invShiftRows(plainText);
    InvSubBytes(plainText);
    InvKeySchedule(chave, i);
    AddRoundKey(plainText, chave);
    MixColumnsInversa(plainText);
    }
    invShiftRows(plainText);
    InvSubBytes(plainText);
    InvKeySchedule(chave, 0);
    AddRoundKey(plainText, chave);
}

byte * Securino::AddRoundKey(byte state[][4], byte roundkey[][4]){
  for (int i = 0; i < 4; i++){
    for(int j=0; j < 4; j++){
      state[i][j] ^= roundkey[i][j];     
    }
  }
}

byte Securino::tableS(byte linha, byte coluna){  
  byte  sBox[16][16] = {
     /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
      {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
      {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
      {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
      {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
      {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
      {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
      {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
      {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
      {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
      {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
      {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
      {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
      {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
      {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
      {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
      {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}
  };
  return sBox[linha][coluna];
}

byte Securino::tableInvS(byte linha, byte coluna){
  byte inv_sBox[16][16] = {
    /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
    {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
    {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
    {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
    {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
    {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
    {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
    {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
    {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
    {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
    {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
    {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
    {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
    {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
    {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
    {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
    {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}
  };
  return inv_sBox[linha][coluna];
}

byte Securino::subX(byte valor){
   byte linha = 0x0;
   byte coluna = 0x0;
   linha = valor/0x10;
   coluna = valor - (linha*0x10);
   return tableS(linha, coluna);
}

byte Securino::invSub(byte valor){
   byte linha = 0x0;
   byte coluna = 0x0;
   linha = valor/0x10;
   coluna = valor - (linha*0x10);
   return tableInvS(linha, coluna);
}

void Securino::subBytes(byte state[][4]){
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      state[i][j] = subX(state[i][j]);
    }
  }
}

void Securino::InvSubBytes(byte state[][4]){
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      state[i][j] = invSub(state[i][j]);
    }
  }
}

void Securino::SubBytesKey(byte chave[]){
  int linha, coluna;
  coluna = 0;
  for (linha = 0; linha < 4; linha++){
    chave[linha] = subX(chave[linha]);
  }
}

void Securino::ShiftRows(byte state[][4]){  
//rotaciona segunda linha da Matriz
  byte aux = state[1][0];
  state[1][0]=state[1][1];
  state[1][1]=state[1][2];
  state[1][2]=state[1][3];
  state[1][3]=aux;

//rotaciona terceira linha da Matriz
  aux = state[2][0];
  state[2][0]=state[2][1];
  state[2][1]=state[2][2];
  state[2][2]=state[2][3];
  state[2][3]=aux;

  aux = state[2][0];
  state[2][0]=state[2][1];
  state[2][1]=state[2][2];
  state[2][2]=state[2][3];
  state[2][3]=aux;

//rotaciona quarta linha da Matriz
  aux = state[3][0];
  state[3][0]=state[3][1];
  state[3][1]=state[3][2];
  state[3][2]=state[3][3];
  state[3][3]=aux;
  
  aux = state[3][0];
  state[3][0]=state[3][1];
  state[3][1]=state[3][2];
  state[3][2]=state[3][3];
  state[3][3]=aux;
  
  aux = state[3][0];
  state[3][0]=state[3][1];
  state[3][1]=state[3][2];
  state[3][2]=state[3][3];
  state[3][3]=aux;
}

void Securino::invShiftRows(byte state [][4]){
  //rotaciona segunda linha da Matriz
  byte aux = state[1][3];
  state[1][3]=state[1][2];
  state[1][2]=state[1][1];
  state[1][1]=state[1][0];
  state[1][0]=aux;

  //rotaciona terceira linha da Matriz
  aux = state[2][3];
  state[2][3]=state[2][2];
  state[2][2]=state[2][1];
  state[2][1]=state[2][0];
  state[2][0]=aux;

  aux = state[2][3];
  state[2][3]=state[2][2];
  state[2][2]=state[2][1];
  state[2][1]=state[2][0];
  state[2][0]=aux;

  //rotaciona quarta linha da Matriz
  aux = state[3][3];
  state[3][3]=state[3][2];
  state[3][2]=state[3][1];
  state[3][1]=state[3][0];
  state[3][0]=aux;
  
  aux = state[3][3];
  state[3][3]=state[3][2];
  state[3][2]=state[3][1];
  state[3][1]=state[3][0];
  state[3][0]=aux;

  aux = state[3][3];
  state[3][3]=state[3][2];
  state[3][2]=state[3][1];
  state[3][1]=state[3][0];
  state[3][0]=aux;
}

//função que guarda os valores de cada posição de 0 a F (em hexadecimal) da tabela E
byte Securino::tableE(byte linha, byte coluna){
  byte tabelaE[0x10][0x10] = {
        /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*0 */   {0x01, 0x03, 0x05, 0x0F, 0x11, 0x33, 0x55, 0xFF, 0x1A, 0x2E, 0x72, 0x96, 0xA1, 0xF8, 0x13, 0x35},
/*1 */   {0x5F, 0xE1, 0x38, 0x48, 0xD8, 0x73, 0x95, 0xA4, 0xF7, 0x02, 0x06, 0x0A, 0x1E, 0x22, 0x66, 0xAA},
/*2 */   {0xE5, 0x34, 0x5C, 0xE4, 0x37, 0x59, 0xEB, 0x26, 0x6A, 0xBE, 0xD9, 0x70, 0x90, 0xAB, 0xE6, 0x31},
/*3 */   {0x53, 0xF5, 0x04, 0x0C, 0x14, 0x3C, 0x44, 0xCC, 0x4F, 0xD1, 0x68, 0xB8, 0xD3, 0x6E, 0xB2, 0xCD},
/*4 */   {0x4C, 0xD4, 0x67, 0xA9, 0xE0, 0x3B, 0x4D, 0xD7, 0x62, 0xA6, 0xF1, 0x08, 0x18, 0x28, 0x78, 0x88},
/*5 */   {0x83, 0x9E, 0xB9, 0xD0, 0x6B, 0xBD, 0xDC, 0x7F, 0x81, 0x98, 0xB3, 0xCE, 0x49, 0xDB, 0x76, 0x9A},
/*6 */   {0xB5, 0xC4, 0x57, 0xF9, 0x10, 0x30, 0x50, 0xF0, 0x0B, 0x1D, 0x27, 0x69, 0xBB, 0xD6, 0x61, 0xA3},
/*7 */   {0xFE, 0x19, 0x2B, 0x7D, 0x87, 0x92, 0xAD, 0xEC, 0x2F, 0x71, 0x93, 0xAE, 0xE9, 0x20, 0x60, 0xA0},
/*8 */   {0xFB, 0x16, 0x3A, 0x4E, 0xD2, 0x6D, 0xB7, 0xC2, 0x5D, 0xE7, 0x32, 0x56, 0xFA, 0x15, 0x3F, 0x41},
/*9 */   {0xC3, 0x5E, 0xE2, 0x3D, 0x47, 0xC9, 0x40, 0xC0, 0x5B, 0xED, 0x2C, 0x74, 0x9C, 0xBF, 0xDA, 0x75},
/*A */   {0x9F, 0xBA, 0xD5, 0x64, 0xAC, 0xEF, 0x2A, 0x7E, 0x82, 0x9D, 0xBC, 0xDF, 0x7A, 0x8E, 0x89, 0x80},
/*B */   {0x9B, 0xB6, 0xC1, 0x58, 0xE8, 0x23, 0x65, 0xAF, 0xEA, 0x25, 0x6F, 0xB1, 0xC8, 0x43, 0xC5, 0x54},
/*C */   {0xFC, 0x1F, 0x21, 0x63, 0xA5, 0xF4, 0x07, 0x09, 0x1B, 0x2D, 0x77, 0x99, 0xB0, 0xCB, 0x46, 0xCA},
/*D */   {0x45, 0xCF, 0x4A, 0xDE, 0x79, 0x8B, 0x86, 0x91, 0xA8, 0xE3, 0x3E, 0x42, 0xC6, 0x51, 0xF3, 0x0E},
/*E */   {0x12, 0x36, 0x5A, 0xEE, 0x29, 0x7B, 0x8D, 0x8C, 0x8F, 0x8A, 0x85, 0x94, 0xA7, 0xF2, 0x0D, 0x17},
/*F */   {0x39, 0x4B, 0xDD, 0x7C, 0x84, 0x97, 0xA2, 0xFD, 0x1C, 0x24, 0x6C, 0xB4, 0xC7, 0x52, 0xF6, 0x01}
  };
   return tabelaE[linha][coluna];
}

//função que guarda os valores de cada posição de 0 a F (em hexadecimal) da tabela L
byte Securino::tableL(byte linha, byte coluna){
  byte tabelaL[0x10][0x10] = {
          /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*0 */    {NULL, 0x00, 0x19, 0x01, 0x32, 0x02, 0x1A, 0xC6, 0x4B, 0xC7, 0x1B, 0x68, 0x33, 0xEE, 0xDF, 0x03},
/*1 */    {0x64, 0x04, 0xE0, 0x0E, 0x34, 0x8D, 0x81, 0xEF, 0x4C, 0x71, 0x08, 0xC8, 0xF8, 0x69, 0x1C, 0xC1},
/*2 */    {0x7D, 0xC2, 0x1D, 0xB5, 0xF9, 0xB9, 0x27, 0x6A, 0x4D, 0xE4, 0xA6, 0x72, 0x9A, 0xC9, 0x09, 0x78},
/*3 */    {0x65, 0x2F, 0x8A, 0x05, 0x21, 0x0F, 0xE1, 0x24, 0x12, 0xF0, 0x82, 0x45, 0x35, 0x93, 0xDA, 0x8E},
/*4 */    {0x96, 0x8F, 0xDB, 0xBD, 0x36, 0xD0, 0xCE, 0x94, 0x13, 0x5C, 0xD2, 0xF1, 0x40, 0x46, 0x83, 0x38},
/*5 */    {0x66, 0xDD, 0xFD, 0x30, 0xBF, 0x06, 0x8B, 0x62, 0xB3, 0x25, 0xE2, 0x98, 0x22, 0x88, 0x91, 0x10},
/*6 */    {0x7E, 0x6E, 0x48, 0xC3, 0xA3, 0xB6, 0x1E, 0x42, 0x3A, 0x6B, 0x28, 0x54, 0xFA, 0x85, 0x3D, 0xBA},
/*7 */    {0x2B, 0x79, 0x0A, 0x15, 0x9B, 0x9F, 0x5E, 0xCA, 0x4E, 0xD4, 0xAC, 0xE5, 0xF3, 0x73, 0xA7, 0x57},
/*8 */    {0xAF, 0x58, 0xA8, 0x50, 0xF4, 0xEA, 0xD6, 0x74, 0x4F, 0xAE, 0xE9, 0xD5, 0xE7, 0xE6, 0xAD, 0xE8},
/*9 */    {0x2C, 0xD7, 0x75, 0x7A, 0xEB, 0x16, 0x0B, 0xF5, 0x59, 0xCB, 0x5F, 0xB0, 0x9C, 0xA9, 0x51, 0xA0},
/*A */    {0x7F, 0x0C, 0xF6, 0x6F, 0x17, 0xC4, 0x49, 0xEC, 0xD8, 0x43, 0x1F, 0x2D, 0xA4, 0x76, 0x7B, 0xB7},
/*B */    {0xCC, 0xBB, 0x3E, 0x5A, 0xFB, 0x60, 0xB1, 0x86, 0x3B, 0x52, 0xA1, 0x6C, 0xAA, 0x55, 0x29, 0x9D},
/*C */    {0x97, 0xB2, 0x87, 0x90, 0x61, 0xBE, 0xDC, 0xFC, 0xBC, 0x95, 0xCF, 0xCD, 0x37, 0x3F, 0x5B, 0xD1},
/*D */    {0x53, 0x39, 0x84, 0x3C, 0x41, 0xA2, 0x6D, 0x47, 0x14, 0x2A, 0x9E, 0x5D, 0x56, 0xF2, 0xD3, 0xAB},
/*E */    {0x44, 0x11, 0x92, 0xD9, 0x23, 0x20, 0x2E, 0x89, 0xB4, 0x7C, 0xB8, 0x26, 0x77, 0x99, 0xE3, 0xA5},
/*F */    {0x67, 0x4A, 0xED, 0xDE, 0xC5, 0x31, 0xFE, 0x18, 0x0D, 0x63, 0x8C, 0x80, 0xC0, 0xF7, 0x70, 0x07}
  };
  return tabelaL[linha][coluna];
}

//função para converter um valor hexadecimal para a tabela L
byte Securino::L(byte valor){
   byte linha = 0x0;
   byte coluna = 0x0;
   linha = valor/0x10;
   coluna = valor - (linha*0x10);
   return tableL(linha, coluna);
}

//função para converter um valor hexadecimal para a tabela E
byte Securino::E(byte valor){
   byte linha = 0x0;
   byte coluna = 0x0;
   linha = (valor/0x10);
   coluna = valor - (linha*0x10);
   return tableE(linha, coluna);
}

byte Securino::calculaLE(byte valor1, byte valor2){
 byte soma =0x00;
 byte resultado = 0x00;
 if(valor1 == 0x00){
   resultado =  0x00;
 }
 else{
   soma = (L(valor1)+L(valor2))%0xFF;
   resultado = E(soma); 
 }
 return resultado;
}

//Aplica a função mix columns
void Securino::MixColumns(byte matrizEntrada[][4]){
   byte aux[4][4];
   byte matrizGalois[4][4] = { 
                              {0x02, 0x03, 0x01, 0x01},
                              {0x01, 0x02, 0x03, 0x01},
                              {0x01, 0x01, 0x02, 0x03},
                              {0x03, 0x01, 0x01, 0x02} 
                            };
  for(int i=0; i < 4; i++){
    for(int j=0; j < 4; j++){
      aux[i][j] = matrizEntrada[i][j]; 
    }
  }
  
  for (int i=0; i < 4; i++){
    matrizEntrada[0][i] = calculaLE(aux[0][i], matrizGalois[0][0]) ^ 
                          calculaLE(aux[1][i], matrizGalois[0][1]) ^ 
                          aux[2][i] ^ 
                          aux[3][i];
                          
    matrizEntrada[1][i] = aux[0][i] ^ 
                          calculaLE(aux[1][i], matrizGalois[1][1]) ^ 
                          calculaLE(aux[2][i], matrizGalois[1][2]) ^ 
                          aux[3][i];
                          
    matrizEntrada[2][i] = aux[0][i] ^ 
                          aux[1][i] ^ 
                          calculaLE(aux[2][i], matrizGalois[2][2]) ^ 
                          calculaLE(aux[3][i], matrizGalois[2][3]);
                          
    matrizEntrada[3][i] = calculaLE(aux[0][i], matrizGalois[3][0]) ^ 
                          aux[1][i] ^ 
                          aux[2][i] ^ 
                          calculaLE(aux[3][i], matrizGalois[3][3]);
  }
}

//função da inversa da mix columns
void Securino::MixColumnsInversa(byte matrizResultante[][4]){
  byte aux[4][4];
  byte matrizGaloisInversa[4][4] = {
                                    {0x0E, 0x0B, 0x0D, 0x09},
                                    {0x09, 0x0E, 0x0B, 0x0D},
                                    {0x0D, 0x09, 0x0E, 0x0B},
                                    {0x0B, 0x0D, 0x09, 0x0E}
                                   };
   for(int i=0; i < 4; i++){
    for(int j=0; j < 4; j++){
      aux[i][j]=matrizResultante[i][j]; 
    }
   }
  
 
  for(int i=0; i < 4; i++){
    matrizResultante[0][i] = calculaLE(aux[0][i], matrizGaloisInversa[0][0]) ^ 
                             calculaLE(aux[1][i], matrizGaloisInversa[0][1]) ^ 
                             calculaLE(aux[2][i], matrizGaloisInversa[0][2]) ^ 
                             calculaLE(aux[3][i], matrizGaloisInversa[0][3]);
    
    matrizResultante[1][i] = calculaLE(aux[0][i], matrizGaloisInversa[1][0]) ^ 
                             calculaLE(aux[1][i], matrizGaloisInversa[1][1]) ^ 
                             calculaLE(aux[2][i], matrizGaloisInversa[1][2]) ^ 
                             calculaLE(aux[3][i], matrizGaloisInversa[1][3]);
    
    matrizResultante[2][i] = calculaLE(aux[0][i], matrizGaloisInversa[2][0]) ^ 
                             calculaLE(aux[1][i], matrizGaloisInversa[2][1]) ^ 
                             calculaLE(aux[2][i], matrizGaloisInversa[2][2]) ^ 
                             calculaLE(aux[3][i], matrizGaloisInversa[2][3]);
    
    matrizResultante[3][i] = calculaLE(aux[0][i], matrizGaloisInversa[3][0]) ^ 
                             calculaLE(aux[1][i], matrizGaloisInversa[3][1]) ^ 
                             calculaLE(aux[2][i], matrizGaloisInversa[3][2]) ^ 
                             calculaLE(aux[3][i], matrizGaloisInversa[3][3]);
  }

}
/*
//https://forum.arduino.cc/index.php?topic=220385.0
//https://www.gta.ufrj.br/grad/10_1/aes/index_files/Page588.htm
*/

void Securino::KeySchedule(byte chave[][4], int round){  
  byte Rcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
  byte copia[4];
  byte matrizcopia[4][4];
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      matrizcopia[i][j] = chave[i][j];
    }
  }
  for(int i=0; i<4; i++){    
      copia[i] = chave[i][3];      
  }
  byte aux = copia[0];
  copia[0] = copia[1];
  copia[1] = copia[2];
  copia[2] = copia[3];
  copia[3] = aux;

  for(int i=0; i<4; i++){
    copia[i] = subX(copia[i]);
  }   
  
   if(round < 9){
    for(int i=0; i<4; i++){
      if(i==0){
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ Rcon[round -1];  
      }else{
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ 0x00;
      }
  }  
   }else if(round == 9){
    for(int i=0; i<4; i++){
      if(i==0){
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ 0x1b;  
      }else{
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ 0x00;
      }
    }
   }else {
    for(int i=0; i<4; i++){
      if(i==0){
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ 0x36;  
      }else{
        matrizcopia[i][0] = chave[i][0] ^ copia[i] ^ 0x00;
      }
   }}

  for(int i=1; i<4; i++){
    matrizcopia[0][i] = chave[0][i] ^ matrizcopia[0][i-1];
    matrizcopia[1][i] = chave[1][i] ^ matrizcopia[1][i-1];
    matrizcopia[2][i] = chave[2][i] ^ matrizcopia[2][i-1];
    matrizcopia[3][i] = chave[3][i] ^ matrizcopia[3][i-1];
  }
for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      chave[i][j] = matrizcopia[i][j];
    }
  }
   
}

void Securino::InvKeySchedule(byte chave[][4], int rodada){  
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      chave[i][j] = keyCopia[i][j];
    }
  }
      
  for(int i=1; i<=rodada; i++)
  {
      KeySchedule(chave, i);
  }
}

void Securino::geraVetorInicializacao(byte ivRandom[])
{
    for(byte i=0x00; i<0x10; i++)
    {
        ivRandom[i] = random(0x00, 0xFF);
    }
}

void Securino::split(byte state[], byte iv[], byte stateIV[]){
  for(int i=0; i<16; i++){
    iv[i] = stateIV[i];
    state[i] = stateIV[i+16];
  }
}
