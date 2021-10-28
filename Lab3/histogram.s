THUMB

  PUBLIC EightBitHistogram
  SECTION .text: CODE
  
  
HistogramReset
  STR R8, [R3], #2
  ADD R7, #1
  CMP R7, #256
  
  BNE HistogramReset
  
  BX LR   
    
    
FillHistogram

  MOV   R3, R9          
  LDRB  R8, [R2], #1    ;Pega o valor da imagem e aumenta R2 = R2 +1 para ir para o próximo pixel
  ADD   R8, R8          ;Multiplica o valor de R8 por 2
  LDR   R10, [R3, R8]   ;Pega o valor atual na posição do vetor, soma 1
  ADD   R10, #1
  ADD   R3, R8          ;Incrementa a posição do vetor no número do pixel e guarda o valor atualizado
  STR   R10, [R3]
  
  ADD   R7, #1          ;Adiciona R7 += 1 e compara se chegou no final da imagem
  CMP   R7, R6
  
  BNE   FillHistogram
  BX    LR
 
EightBitHistogram
  MUL   R6, R0, R1      ;Guarda em R6 o tamanho da imagen
  MOV   R9, R3          ;Guarda em R9 a posição inicial do vetor

  CMP   R6, #65536      ;Compara se a imagem é maior do que 64k e se for retorna 0
  ITT   GT
    MOVGT R0, #0
    BXGT  LR
      
  MOV   R7, #0          ;Prepara os registradores e zera o vetor
  MOV   R8, #0
  PUSH  {LR}
  BL HistogramReset
  POP   {LR}
    
  MOV   R7, #0          ;Zera o R7 e preenche o vetor
  PUSH {LR}
  BL FillHistogram
  POP {LR}
    
  MOV   R0, R6          ;Guarda o tamanho da imagem em R0 e retorna

  BX    LR
	
  END