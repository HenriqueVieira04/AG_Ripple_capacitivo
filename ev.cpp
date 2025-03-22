#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <vector>
#include <GL/glut.h>


//criando os randomizadores usando mt19937 
using namespace std;
random_device rd1;
random_device rd2;
mt19937 gen1(rd1());
mt19937 gen2(rd2());

// variaveis gerais de simulação
double Vs = 150.0; // Tensao da fonte
double R = 630.0; // Resistencia do circuito dada pelo usuario
// Capaciatancia é dada em uF ou seja, precisa de pow(10, -6);
double F = 120; // Frequencia de onsilação retiificada

double qtdpassos = 2000; // = periodo/dx
double passorad = M_PI / qtdpassos; // = dx
double passosec = 1 / (qtdpassos * F); // = incremento do tempo em que estamos em determinado passo

float percent_min_taget = 0.1;

double ripple_target = Vs / (1 + percent_min_taget);

float mutation_qtd = 1.2f;
float mutation_arr = 1;

int population;
bool lock = false;

bool nearlyEqual(float a, float b, float epsilon = 1e-5f) {
    return fabs(a - b) < epsilon;
}



vector<float> e24_values = { 
    1.0e-6f, 1.1e-6f, 1.2e-6f, 1.3e-6f, 1.5e-6f, 1.6e-6f, 1.8e-6f, 2.0e-6f, 2.2e-6f, 2.4e-6f, 2.7e-6f, 3.0e-6f, 
    3.3e-6f, 3.6e-6f, 3.9e-6f, 4.3e-6f, 4.7e-6f, 5.1e-6f, 5.6e-6f, 6.2e-6f, 6.8e-6f, 7.5e-6f, 8.2e-6f, 9.1e-6f, 
    1.0e-5f, 1.1e-5f, 1.2e-5f, 1.3e-5f, 1.5e-5f, 1.6e-5f, 1.8e-5f, 2.0e-5f, 2.2e-5f, 2.4e-5f, 2.7e-5f, 3.0e-5f, 
    3.3e-5f, 3.6e-5f, 3.9e-5f, 4.3e-5f, 4.7e-5f, 5.1e-5f, 5.6e-5f, 6.2e-5f, 6.8e-5f, 7.5e-5f, 8.2e-5f, 9.1e-5f, 
    1.0e-4f, 1.1e-4f, 1.2e-4f, 1.3e-4f, 1.5e-4f, 1.6e-4f, 1.8e-4f, 2.0e-4f, 2.2e-4f, 2.4e-4f, 2.7e-4f, 3.0e-4f, 
    3.3e-4f, 3.6e-4f, 3.9e-4f, 4.3e-4f, 4.7e-4f, 5.1e-4f, 5.6e-4f, 6.2e-4f, 6.8e-4f, 7.5e-4f, 8.2e-4f, 9.1e-4f, 
    1.0e-3f, 1.1e-3f, 1.2e-3f, 1.3e-3f, 1.5e-3f, 1.6e-3f, 1.8e-3f, 2.0e-3f, 2.2e-3f, 2.4e-3f, 2.7e-3f, 3.0e-3f, 
    3.3e-3f, 3.6e-3f, 3.9e-3f, 4.3e-3f, 4.7e-3f, 5.1e-3f, 5.6e-3f, 6.2e-3f, 6.8e-3f, 7.5e-3f, 8.2e-3f, 9.1e-3f, 
    1.0e-2f, 1.1e-2f, 1.2e-2f, 1.3e-2f, 1.5e-2f, 1.6e-2f, 1.8e-2f, 2.0e-2f, 2.2e-2f, 2.4e-2f, 2.7e-2f, 3.0e-2f, 
    3.3e-2f, 3.6e-2f, 3.9e-2f, 4.3e-2f, 4.7e-2f, 5.1e-2f, 5.6e-2f, 6.2e-2f, 6.8e-2f, 7.5e-2f, 8.2e-2f, 9.1e-2f, 
    1.0e-1f, 1.1e-1f, 1.2e-1f, 1.3e-1f, 1.5e-1f, 1.6e-1f, 1.8e-1f, 2.0e-1f, 2.2e-1f, 2.4e-1f, 2.7e-1f, 3.0e-1f, 
    3.3e-1f, 3.6e-1f, 3.9e-1f, 4.3e-1f, 4.7e-1f, 5.1e-1f, 5.6e-1f
};


int nextQtd(int qtd, double mutation_qtd){
    normal_distribution<> distribNormal1(qtd, mutation_qtd);
    int newQtd = distribNormal1(gen1);
    return clamp(newQtd, 1, 6);
}

vector<float> nextArr(int qtd, const vector<int>& oldarray, float mutation_arr){
    vector<float> new_array(qtd); //tem que mexer nessa poha
    int newindex = 0;
    for(int cont = 0; cont < qtd; cont++){
        normal_distribution<> distribNormal2(oldarray[cont], mutation_arr);
        newindex = distribNormal2(gen2);
        newindex = clamp(newindex, 0, 139);
        new_array[cont] = e24_values[newindex];
    }
    return new_array;
}

float sumcap(const vector<float>& arr){
    float sum = 0;
    for(float val : arr){
        sum += val;
    }
    return sum;
}

double findW(const vector<float>& arr) {
    double w = 0;
    double C = sumcap(arr);
    double exp_value, sin_value;
    double initialTime = 1.0 / (2 * F); // Tempo inicial para a função exponencial
    
    // Itera sobre os valores de radianos
    for (double p = 0; p <= M_PI / 2; p += passorad) {
        exp_value = Vs * exp((-1 * initialTime) / (R * C)); // Convertendo radianos para tempo
        sin_value = Vs * sin(p);
        initialTime += passosec;

        if (sin_value > exp_value) {
            w = p;
            break; // Interrompe quando encontramos o valor em que sin_value é maior
        }
    }
    return w;
}

double integrateTrapezoid_senoid(double (*func)(double), double start, double end, double step) {
    double sum_of_parts = 0;
    for (double x = start; x < end; x += step) {
        sum_of_parts += 0.5 * (func(x) + func(x + step)) * step;
    }
    return sum_of_parts;
}

double calcvy(double taux, double C, int cont) {
    double exp_value = Vs * exp((-1 * taux) / (R * C));
    double sin_value = Vs * sin((M_PI / 2) - (cont * passorad));
    if (exp_value > sin_value)
        return exp_value;
    else
        return sin_value;
}

double find_Vmed(double w, const vector<float>& arr){
    double integral_acum = 0;
    double it_sec = 0;
    double C = sumcap(arr);
    double taux = 0;


    for(double it_cont = 0.5*M_PI; it_cont < M_PI+w; it_cont+=passorad){ //integral da parte do ripple
        integral_acum += Vs * exp((-1 * taux) / (R * C)) * (passorad);
        taux += passosec;
    }


    for(;w < 0.5*M_PI; w+=passorad) //integral da parte do ripple
        integral_acum += Vs * fabs(sin(w)) * (passorad);

    return integral_acum/M_PI;
} 


class Cenario{
    public:
        int qtd_cap;
        int price = 0;
        vector<float> cap_array;
        float score;
        float Ctotal;

        double w;
        double Vmed;
        double Vcut;

    public: 
        Cenario(int qtd, const vector<float>& arr){
            qtd_cap = qtd;
            cap_array = arr;
            w = findW(cap_array);
            Vmed = find_Vmed(w, cap_array);
            Vcut = Vs * fabs(sin(M_PI+w));
            price = cap_Price();
            score = avl_fitness();
            Ctotal = sumcap(cap_array);
        }
        
        int search_for_it(float value) {
            int cont; 
            for (cont = 0; cont < e24_values.size(); cont++) {
                if (nearlyEqual(e24_values[cont], value)) {
                    break;
                }
            }
            if (cont < e24_values.size()) {
                return (cont / 14 + 1);
            }
            std :: cout << "n achei" <<std::endl;
            return -1; // Caso não encontre o valor
        }
        

        float avl_fitness(){ //int qtd, float* arr
            return (ripple_target-Vcut)/price;
        }

        int cap_Price(){ //int qtd, float* arr
            int price_aux = 0;
            for(int cont = 0; cont < qtd_cap; cont++) 
                price_aux += search_for_it(cap_array[cont]);
            return price_aux;
        }

        ~Cenario(){}

        vector<int> indexs(int qtd, const vector<float>& oldarray) {
            vector<int> indices;
            indices.reserve(oldarray.size());
            for (int i = 0; i < oldarray.size(); ++i) {
                int idx = -1;
                for (int j = 0; j < e24_values.size(); ++j) {
                    if (e24_values[j] == oldarray[i]) {
                        idx = j;
                        break;
                    }
                }
                indices.push_back(idx);
            }
            return indices;
        }

        Cenario Evolve(){
            Cenario cen_new(nextQtd(qtd_cap, mutation_qtd), nextArr(qtd_cap, indexs(qtd_cap, cap_array), mutation_arr));
            return cen_new;
        }
    
};

Cenario best_cenario(1, vector<float>(1, 1.0e-4f));
vector<Cenario> cenarios; //cria um vetor de cenarios

float generate_wave = 10; // geracao dos periodos de onda na GUI
float zoomFactor = 1.0f; // fator de zoom default

// Variáveis de controle de rolagem
float scrollX = 0.0f; 
float scrollY = 0.0f; 
float scrollSpeed = 0.1f; // Velocidade de rolagem

int scrollcount = 0; // quantidade de rolagem para geração de ondas
int aux_drawlines = 0;

//contagem para desenho da escala nos eixos
int contw = 0;
int conts = 0;

void renderBitmapString(float x, float y, void *font, const char *string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string++);
    }
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-0.1 * M_PI * zoomFactor + scrollX, 10 * M_PI * zoomFactor + scrollX,
            -1.5 * zoomFactor + scrollY, 1.5 * zoomFactor + scrollY);
    glMatrixMode(GL_MODELVIEW);
    glLineWidth(2.0);
}

void tela() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-scrollX, -scrollY, 0.0f);

   
    for (int k = 0; k < generate_wave; k++) {     
        double taux = 0;
        int cont = 0;
        double valuey = 0;

        glBegin(GL_LINE_STRIP);
        glColor3f(1.0, 1.0, 1.0);
        for (double p2 = (M_PI / 2) + k * M_PI; p2 <= (M_PI + best_cenario.w) + k * M_PI; p2 += passorad) {
            valuey = calcvy(taux, best_cenario.Ctotal, cont);
            //printf("%lf\n", valuey);
            cont++;
            glVertex2f(p2, valuey);
            taux += passosec;
        }
        glEnd();
        
        double start = (M_PI + best_cenario.w) + k * M_PI;
        double end = (1.5 * M_PI) + k * M_PI;

        glBegin(GL_LINE_STRIP);
        glColor3f(1.0, 1.0, 1.0);
        for (double p3 = start; p3 <= end; p3 += passorad) 
            glVertex2f(p3, Vs * fabs(sin(p3)));
        
        glEnd();

        renderBitmapString(k*M_PI-0.15, -0.1, GLUT_BITMAP_HELVETICA_12, (std::to_string(k)+"pi").c_str());
        renderBitmapString(k*M_PI+M_PI/2-0.35, -0.1, GLUT_BITMAP_HELVETICA_12, (std::to_string(((int)k)*2+1)+"/2 pi").c_str());

        if (aux_drawlines == 0) {
            std::cout << "Tensão média do circuito: " << best_cenario.Vmed << "V" << std::endl;
            std::cout << "Tensão minima para corte: " << best_cenario.Vcut << "V" << std::endl;
            std::cout << "Score inicial: " << best_cenario.score << std::endl;
            aux_drawlines++;
        }
    }


    // Linha da tensão média
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0); // Cor vermelha para o eixo X
    glVertex2f(-5 * generate_wave, 0.0);
    glVertex2f(5 * generate_wave, 0.0);

    glColor3f(0.5, 0.0, 0.5);
    glVertex2f(-5 * generate_wave, Vs);
    glVertex2f(5 * generate_wave, Vs);

    glColor3f(0.0, 1.0, 0.0);
    glVertex2f(-5 * generate_wave, best_cenario.Vmed);
    glVertex2f(5 * generate_wave, best_cenario.Vmed);

    glColor3f(1.0, 1.0, 0.0);
    glVertex2f(-5 * generate_wave, best_cenario.Vcut);
    glVertex2f(5 * generate_wave, best_cenario.Vcut);

    glColor3f(1.0, 0.0, 0.0); // Cor vermelha para o eixo Y
    glVertex2f(0.0, (-2 * Vs - M_PI * conts));
    glVertex2f(0.0, (2 * Vs + M_PI * contw));

    glBegin(GL_LINE_STRIP);
    glColor3f(1.0, 1.0, 1.0);
    for (double p1 = 0; p1 <= M_PI / 2; p1 += passorad) {
        glVertex2f(p1, Vs * sin(p1));
    }
    glEnd();
    

    glColor3f(1.0, 1.0, 1.0); // Cor branca para o texto
    renderBitmapString(0.1, best_cenario.Vmed, GLUT_BITMAP_HELVETICA_12, ("Vmed: " + std::to_string(best_cenario.Vmed)+"V").c_str());
    renderBitmapString(0.1, best_cenario.Vcut, GLUT_BITMAP_HELVETICA_12, ("Vcorte: " + std::to_string(best_cenario.Vcut)+"V").c_str());
    renderBitmapString(0.1, Vs, GLUT_BITMAP_HELVETICA_12, ("Vmax: " + std::to_string(Vs)+"V").c_str());

    glFlush();
}

void zoomIn() {
    zoomFactor /= 1.1f;
    generate_wave /= 1.1;
    generate_wave = ceil(generate_wave);
    init();
    glutPostRedisplay();
}

void zoomOut() {
    zoomFactor *= 1.1f;
    generate_wave *= 1.1;
    generate_wave = ceil(generate_wave);
    init();
    glutPostRedisplay();
}

void teclado(unsigned char key, int x, int y) {
    switch (key) {
        case '+':
            zoomIn();
            break;
        case '-':
            zoomOut();
            break;
        case 'a': // Rolagem para a esquerda
            scrollX -= scrollSpeed;
            scrollcount--;
            if (scrollcount == -15) {
                scrollcount = 0;
                generate_wave /= 1.1;
                generate_wave = ceil(generate_wave);
                //init();
                glutPostRedisplay();
            }
            //init();
            glutPostRedisplay();
            break;
        case 'd': // Rolagem para a direita
            scrollX += scrollSpeed;
            scrollcount++;
            if (scrollcount == 15) {
                scrollcount = 0;
                generate_wave *= 1.1;
                generate_wave = ceil(generate_wave);
                glutPostRedisplay();
            }
            glutPostRedisplay();
            break;
        case 'w': // Rolagem para cima
            scrollY += scrollSpeed;
            contw++;
            glutPostRedisplay();
            break;
        case 's': // Rolagem para baixo
            scrollY -= scrollSpeed;
            conts++;
            glutPostRedisplay();
            break;
        case ' ':
            scrollX = 0.0f;
            scrollY = 0.0f;
            glutPostRedisplay();
            break;
        case 'g': //continuar codando essa parte
            lock = !lock;
            std::cout << "Lock: " << lock << std::endl;
            if(lock == false){
                for(int cont = 0; cont < best_cenario.qtd_cap; cont++){
                    std::cout << "Capacitor " << cont << ": " << best_cenario.cap_array[cont] << std::endl;
                }
            }             
            break;
        case 'm':
            mutation_arr *=1.05;
            std::cout << "mutation_arr: " << mutation_arr << std::endl; 
            break;
        case 'n':
            mutation_arr /=1.05;
            std::cout << "mutation_arr: " << mutation_arr << std::endl;
            break;
        case 'p':
            mutation_qtd *=1.05;
            std::cout << "mutation_qtd: " << mutation_qtd << std::endl;
            break;
        case 'o':
            mutation_qtd /=1.05;
            std::cout << "mutation_qtd: " << mutation_qtd << std::endl;
            break;
        default:
            break;
    }
}

void idleCallback() {
    if (lock) {
        // Evolução da população
        for(int cont = 0; cont < population; cont++){
            if (cenarios[cont].score < best_cenario.score){
                best_cenario = cenarios[cont];
                cont++; // protege o individuo
            }
            cenarios[cont] = cenarios[cont].Evolve();
            std :: cout << "-------------------------------" << std :: endl;
            std :: cout << "estou evoluindo: " << best_cenario.score << std :: endl;
            std :: cout << "Tensão minima para corte: " << best_cenario.Vcut << "V" << std::endl;
            std :: cout << "Preço: " << best_cenario.price << std::endl;
            std :: cout << "-------------------------------" << std :: endl;
        }
        // Atualiza a tela
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {  

    population = 1000; // preenche o tamanho da população

    int qtdaux = 0; //variavel auxiliar para gerar tamanhos diferentes

    for(int cont = 0; cont < population; cont++){ //cria uma população base com tamanho diferentes mas com elementos iguais
        qtdaux = nextQtd(3, mutation_qtd); 
        cenarios.push_back(Cenario(
            qtdaux, 
            nextArr(qtdaux, vector<int>(qtdaux, 70), mutation_arr) //preenche a população com vectors contendo elementos ja alatorizados
        ));
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Grafico Ripple");
    glutDisplayFunc(tela);
    glutKeyboardFunc(teclado);

    glutIdleFunc(idleCallback);

    init();
    glutMainLoop();

    return 0;
}

//--------------------------------------------------------------------------------------------------------------