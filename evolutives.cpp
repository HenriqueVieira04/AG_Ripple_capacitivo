#include "capacitors.cpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <GL/glut.h>
#include <tuple>
#include <vector>

//criando os randomizadores usando mt19937 
using namespace std;
random_device rd1;
random_device rd2;
mt19937 gen1(rd1());
mt19937 gen2(rd2());

// variaveis gerais de simulação
double Vs = 24.0; // Tensao da fonte
double R = 1.0; // Resistencia do circuito dada pelo usuario
// Capaciatancia é dada em uF ou seja, precisa de pow(10, -6);
double F = 120; // Frequencia de onsilação retiificada

double qtdpassos = 2000; // = periodo/dx
double passorad = M_PI / qtdpassos; // = dx
double passosec = 1 / (qtdpassos * F); // = incremento do tempo em que estamos em determinado passo


//porcentagem minima desejada de ripple
float percent_min_taget = 0.1;

double ripple_target = Vs / (1 + percent_min_taget);

float mutation_qtd = 1.2f;
float mutation_arr = 1;

int population;
bool lock = false;

bool nearlyEqual(float a, float b, float epsilon = 1e-5f) {
    return fabs(a - b) < epsilon;
}

vector<tuple<double, float>> caps = initCaps();


int nextQtd(int qtd, double mutation_qtd){
    normal_distribution<> distribNormal1(qtd, mutation_qtd);
    int newQtd = distribNormal1(gen1);
    return clamp(newQtd, 1, 6);
}

vector<tuple<double, float>> nextArr(int qtd, const vector<int>& oldarray, float mutation_arr){
    vector<tuple<double, float>> new_array(qtd);
    int newindex = 0;
    uniform_int_distribution<> distribUniform(0, caps.size() - 1); // Para gerar índices aleatórios

    for(int cont = 0; cont < qtd; cont++){
        int old_index;
        if (cont < oldarray.size()) {
            // Se houver um capacitor "pai", baseia-se nele
            old_index = oldarray[cont];
        } else {
            // Se for um capacitor novo, gera um índice aleatório
            old_index = distribUniform(gen2);
        }

        normal_distribution<> distribNormal2(old_index, mutation_arr);
        newindex = distribNormal2(gen2);
        newindex = clamp(newindex, 0, (int)caps.size() - 1);
        new_array[cont] = caps[newindex];
    }
    return new_array;
}

float sumcap(const vector<tuple<double, float>>& arr){
    float sum = 0;
    for(const auto& val : arr){
        sum += get<0>(val);
    }
    return sum;
}

double findW(const vector<tuple<double, float>>& arr) {
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

double find_Vmed(double w, const vector<tuple<double, float>>& arr){
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
        float price = 0;
        vector<tuple<double, float>> cap_array;
        float score;
        float Ctotal;

        double w;
        double Vmed;
        double Vcut;

    public: 
        Cenario(int qtd, const vector<tuple<double, float>>& arr){
            qtd_cap = qtd;
            cap_array = arr;
            w = findW(cap_array);
            Vmed = find_Vmed(w, cap_array);
            Vcut = Vs * fabs(sin(M_PI+w));
            price = cap_Price();
            score = avl_fitness();
            Ctotal = sumcap(cap_array);
        }
        
        float avl_fitness(){ //int qtd, float* arr
            return (ripple_target-Vcut)/price;
        }

        float cap_Price(){ //int qtd, float* arr
            float price_aux = 0;
            for(int cont = 0; cont < qtd_cap; cont++) 
                price_aux += get<1>(cap_array[cont]);
            return price_aux;
        }

        ~Cenario(){}

        vector<int> indexs() {
            vector<int> indices;
            indices.reserve(cap_array.size());
            for (const auto& cap_tuple : cap_array) {
                double target_cap_value = get<0>(cap_tuple);
                int idx = -1;
                for (int j = 0; j < caps.size(); ++j) {
                    if (nearlyEqual(get<0>(caps[j]), target_cap_value)) {
                        idx = j;
                        break;
                    }
                }
                indices.push_back(idx);
            }
            return indices;
        }

        Cenario Evolve(){
            int new_qtd = nextQtd(qtd_cap, mutation_qtd);
            Cenario cen_new(new_qtd, nextArr(new_qtd, indexs(), mutation_arr));
            return cen_new;
        }
    
};

Cenario best_cenario(1, {make_tuple(1.0e-4f, 0.15f)});
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
    glEnable(GL_LINE_SMOOTH); // Habilita o anti-aliasing para linhas
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

    // --- Etapa 1: Calcular todos os vértices da onda e armazenar em um vetor ---
    std::vector<std::pair<double, double>> wave_vertices;

    // Adiciona o primeiro segmento da onda (senoide inicial)
    for (double p1 = 0; p1 <= M_PI / 2; p1 += passorad) {
        wave_vertices.push_back({p1, Vs * sin(p1)});
    }

    // Adiciona os períodos de ripple
    for (int k = 0; k < generate_wave; k++) {     
        double taux = 0;
        int cont = 0;
        
        // Parte da descarga do capacitor (curva exponencial)
        for (double p2 = (M_PI / 2) + k * M_PI; p2 <= (M_PI + best_cenario.w) + k * M_PI; p2 += passorad) {
            double valuey = calcvy(taux, best_cenario.Ctotal, cont);
            wave_vertices.push_back({p2, valuey});
            cont++;
            taux += passosec;
        }
        
        // Parte da recarga (curva senoidal)
        double start = (M_PI + best_cenario.w) + k * M_PI;
        double end = (1.5 * M_PI) + k * M_PI;
        for (double p3 = start; p3 <= end; p3 += passorad) {
            wave_vertices.push_back({p3, Vs * fabs(sin(p3))});
        }
    }

    // Desenha a linha contínua que conecta todos os pontos
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0, 1.0, 1.0); // Cor branca para a onda
    for (const auto& vertex : wave_vertices) {
        glVertex2d(vertex.first, vertex.second);
    }
    glEnd();

    // Desenha os pontos individuais para visualização
    glPointSize(3.0f); // Define o tamanho dos pontos
    glBegin(GL_POINTS);
    glColor3f(0.0, 1.0, 1.0); // Cor ciano para os pontos, para destacá-los
    for (const auto& vertex : wave_vertices) {
        glVertex2d(vertex.first, vertex.second);
    }
    glEnd();


    // Desenha as marcações de PI no eixo X
    for (int k = 0; k < generate_wave; k++) {
        renderBitmapString(k*M_PI-0.15, -0.1, GLUT_BITMAP_HELVETICA_12, (std::to_string(k)+"pi").c_str());
        renderBitmapString(k*M_PI+M_PI/2-0.35, -0.1, GLUT_BITMAP_HELVETICA_12, (std::to_string(((int)k)*2+1)+"/2 pi").c_str());
    }

    if (aux_drawlines == 0) {
        std::cout << "Tensão média do circuito: " << best_cenario.Vmed << "V" << std::endl;
        std::cout << "Tensão minima para corte: " << best_cenario.Vcut << "V" << std::endl;
        std::cout << "Score inicial: " << best_cenario.score << std::endl;
        aux_drawlines++;
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
    glEnd();
    

    glColor3f(1.0, 1.0, 1.0); // Cor branca para o texto
    renderBitmapString(0.1, best_cenario.Vmed, GLUT_BITMAP_HELVETICA_12, ("Vmed: " + std::to_string(best_cenario.Vmed)+"V").c_str());
    renderBitmapString(0.1, best_cenario.Vcut, GLUT_BITMAP_HELVETICA_12, ("Vcorte: " + std::to_string(best_cenario.Vcut)+"V").c_str());
    renderBitmapString(0.1, Vs, GLUT_BITMAP_HELVETICA_12, ("Vmax: " + std::to_string(Vs)+"V").c_str());

    // Exibe os pesos de mutação no canto superior direito
    /*
    string mutation_arr_str = "Mutation Arr: " + std::to_string(mutation_arr);
    string mutation_qtd_str = "Mutation Qtd: " + std::to_string(mutation_qtd);
    renderBitmapString(8 * M_PI * zoomFactor + scrollX, 1.4 * zoomFactor + scrollY, GLUT_BITMAP_HELVETICA_12, mutation_arr_str.c_str());
    renderBitmapString(8 * M_PI * zoomFactor + scrollX, 1.3 * zoomFactor + scrollY, GLUT_BITMAP_HELVETICA_12, mutation_qtd_str.c_str());
    */
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

void autoFitView() {
    // Calcula os limites da onda no eixo X
    float x_min = 0.0f;
    float x_max = (1.5f * M_PI) + (generate_wave - 1) * M_PI;

    // Calcula os limites da onda no eixo Y
    // O valor máximo é a tensão da fonte (Vs) e o mínimo é a tensão de corte.
    float y_min = best_cenario.Vcut;
    float y_max = Vs;

    // Adiciona uma pequena margem para que a onda não fique colada nas bordas
    float x_margin = (x_max - x_min) * 0.05f; // 5% de margem
    float y_margin = (y_max - y_min) * 0.10f; // 10% de margem

    // Reseta a rolagem
    scrollX = 0.0f;
    scrollY = 0.0f;

    // Ajusta a projeção do OpenGL para enquadrar a onda com as margens
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(x_min - x_margin, x_max + x_margin, y_min - y_margin, y_max + y_margin);
    glMatrixMode(GL_MODELVIEW);

    // Solicita que a tela seja redesenhada
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
                    std::cout << "Capacitor " << cont << ": " << get<0>(best_cenario.cap_array[cont]) << "F, R$" << get<1>(best_cenario.cap_array[cont]) << std::endl;
                }
            }             
            break;
        case 'm':
            mutation_arr *=1.05;
            std::cout << "mutation_arr: " << mutation_arr << std::endl; 
            glutPostRedisplay();
            break;
        case 'n':
            mutation_arr /=1.05;
            std::cout << "mutation_arr: " << mutation_arr << std::endl;
            glutPostRedisplay();
            break;
        case 'p':
            mutation_qtd *=1.05;
            std::cout << "mutation_qtd: " << mutation_qtd << std::endl;
            glutPostRedisplay();
            break;
        case 'o':
            mutation_qtd /=1.05;
            std::cout << "mutation_qtd: " << mutation_qtd << std::endl;
            glutPostRedisplay();
            break;
        case 'f': // Tecla para auto-ajuste
            autoFitView();
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
            nextArr(qtdaux, vector<int>(), mutation_arr) //preenche a população com vectors contendo elementos ja alatorizados
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