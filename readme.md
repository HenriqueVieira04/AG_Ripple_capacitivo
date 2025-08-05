ESTE REPOSITÓRIO CONTÉM UMA IMPLEMENTAÇÃO DE UM ALGORITMO EVOLUTIVO UTILIZADO PARA ENCONTRAR UMA COMBINAÇÃO IDEAL DE CAPACITORES VISANDO OBTER O MENOR RIPPLE CAPACITIVO POSSÍVEL, TENDO EM VISTA UMA FUNÇÃO DE CUSTO PARA CADA CAPACITOR ESCOLHIDO. O CRITÉRIO PARA A EVOLUÇÃO DOS ESPÉCIMES (CENÁRIOS COM COMBINAÇÕES DE CAPACITORES) É O CUSTO-BENEFÍCIO.

**Aluno:** Henrique Vieira Lima
**Professor:** Eduardo do Valle Simões

**Oferecimento:** CNPq - Conselho Nacional de Desenvolvimento Científico e Tecnológico

---

### Como Compilar e Executar

O projeto utiliza `g++` e as bibliotecas gráficas `OpenGL` e `GLUT`. Certifique-se de que elas estejam instaladas no seu sistema.

**Dependências (Debian/Ubuntu):**
```bash
sudo apt-get update
sudo apt-get install build-essential freeglut3-dev
```

Para compilar, utilize o makefile fornecido:
```bash
make
```

Para executar o programa:
```bash
make run
```
Ou diretamente:
```bash
./evolutives
```

Para limpar os arquivos compilados:
```bash
make clean
```

---

### Como Utilizar

A interface gráfica exibe a forma de onda da tensão no capacitor. Você pode interagir com a simulação usando as seguintes teclas:

-   **`g`**: Inicia ou pausa o processo de evolução do algoritmo genético.
-   **`f`**: Ajusta automaticamente o zoom e a posição para enquadrar a onda na tela.
-   **`+` / `-`**: Aumenta ou diminui o zoom.
-   **`w` / `s`**: Move a visualização para cima ou para baixo.
-   **`a` / `d`**: Move a visualização para a esquerda ou para a direita.
-   **`espaço`**: Reseta a visualização para a posição inicial.

---

### Parâmetros

#### Parâmetros do Circuito
No início do código `evolutives.cpp`, encontram-se três variáveis responsáveis pelo ajuste do cenário do circuito. Faça as alterações conforme julgar necessário:
-   `Vs`: Tensão da fonte.
-   `R`: Carga resistiva do circuito.
-   `F`: Frequência da rede.

#### Parâmetros de Evolução
Os seguintes parâmetros do algoritmo evolutivo podem ser ajustados em tempo real durante a execução:
-   **`m` / `n`**: Aumenta ou diminui o peso da mutação sobre os **valores** dos capacitores (`mutation_arr`).
-   **`p` / `o`**: Aumenta ou diminui o peso da mutação sobre a **quantidade** de capacitores (`mutation_qtd`).

Os valores atuais desses parâmetros são exibidos no canto superior esquerdo da tela.

**!!Não se recomenda a alteração de outras variáveis contidas no código!!**

