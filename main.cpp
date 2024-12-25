#include <GL/freeglut.h>
#include <string>
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

// Переменные для перемещения по сцене и анимации
float X = -4.0, Z = 550.0, Y = 30.0; // Позиция камеры
float X0 = 0.0, Z0 = -1.0;
float angle = 0.0f;
float moveLR = 0.0f;
float moveAB = 0.0f;
float moveUp = 0.0f;
bool anim = false;

// Узел списка
template<typename t>
class Node {
private:
	t info; // Информация в узле списка
	Node* next; // Указатель на следующий узел списка

public:

	// Геттеры - сеттеры
	t& Info() {
		return info;
	}
	void Info(t info) {
		this->info = info;
	}
	Node* getNext() const {
		return next;
	}
	void setNext(Node* next) {
		this->next = next;
	}

	// Конструктор
	Node(t info) {
		this->info = info;
		next = nullptr;
	}
};

// Связный список
template<typename t>
class List {
private:

	// Голова списка
	Node<t>* head;
	int size; // Размер списка

public:

	// Геттеры
	Node<t>* getHead() const {
		return head;
	}
	int getSize() const {
		return size;
	}

	// Конструктор
	List() {
		head = nullptr;
		size = 0;
	}

	// Добавляем новый узел в список
	void push_back(t info) {
		Node<t>* tmp = new Node<t>(info);
		if (head) {
			tmp->setNext(head);
		}
		head = tmp;
		size++;
	}

	// Очистка списка
	void clear() {
		while (head) {
			Node<t>* tmp = head->getNext();
			delete head;
			head = tmp;
		}
		head = nullptr;
		size = 0;
	}

	// Деструктор
	~List() {
		clear();
	}
};

// Текстура
class Texture {
private:
	GLuint texture = 0; // Текстура
	BYTE* data = nullptr; // Изображение
	int width, height; // Разрешение

public:

	// Геттеры
	GLuint getTexture() const {
		return texture;
	}
	BYTE* getData() const {
		return data;
	}
	int getWidth() const {
		return width;
	}
	int getHeight() const {
		return height;
	}

	// По умолчанию
	Texture() {
		texture = 0;
		data = nullptr;
		width = height = 0;
	}

	// Конструктор
	Texture(string path, int width, int height) {
		this->width = width;
		this->height = height;
		loadTexture(path);
	}

	// Конструктор копирования
	Texture(const Texture& texture) {
		*this = texture;
	}

	// Оператор присваивания
	Texture& operator=(const Texture& texture) {
		if (this != &texture) {
			this->texture = texture.texture;
			this->width = texture.width;
			this->height = texture.height;
			this->data = (BYTE*)malloc(width * height * 3);
			for (int i = 0; i < this->width * this->height * 3; i++) {
				this->data[i] = texture.data[i];
			}
		}
		return *this;
	}

	// Загрузка текстуры
	void loadTexture(string path) {
		FILE* file = NULL;

		// Открываем файл для побитового чтени¤
		fopen_s(&file, path.c_str(), "rb");

		// Ошибка открытия файла
		if (!file) {
			cout << "Can't open file with texture!" << endl;
		}

		// Выделяем память под текстуру
		data = (BYTE*)malloc(width * height * 3);

		// Читаем побитово изображение
		fread(data, width * height * 3, 1, file);
		fclose(file);
	}

	// Привязываем текстуру
	void add() {
		glPushMatrix();
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_TEXTURE_ENV_COLOR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glEnable(GL_TEXTURE_2D);
	}

	// Отвязываем текстуру
	void remove() {
		glDisable(GL_TEXTURE_2D);
		glDeleteTextures(1, &texture);
		glPopMatrix();
	}

	// Отрисовка
	virtual void draw() = 0;

	// Деструктор
	~Texture() {
		if (data) {
			free(data);
		}
	}
};

// Координата в 3-х мерном пространстве
class Point3D {
private:
	float x, y, z; // Координаты

public:

	// Конструкторы
	Point3D() {
		x = y = z = 0;
	}
	Point3D(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	// Геттеры - сеттеры
	float getX() const {
		return x;
	}
	float getY() const {
		return y;
	}
	float getZ() const {
		return z;
	}
	void setX(float x) {
		this->x = x;
	}
	void setY(float y) {
		this->y = y;
	}
	void setZ(float z) {
		this->z = z;
	}
};

// Планета
class Planet : public Texture {
private:
	float speed, timer; // Скорость на орбите
	float size; // Коэффициент для размера планет
	Point3D pos; // Положение

public:

	// Геттеры
	float getSpeed() const {
		return speed;
	}
	float getTimer() const {
		return timer;
	}
	float getSize() const {
		return size;
	}
	Point3D getPos() const {
		return pos;
	}

	// Конструкторы
	Planet() : Texture() {
		speed = timer = size = 0;
	}
	Planet(string path, int width, int height, float speed, float size, Point3D pos) 
		: Texture(path, width, height) {
		this->speed = speed;
		this->pos = pos;
		this->size = size;
		timer = 0;
	}

	// Отрисовка
	void draw() override {
		add();

		GLUquadric* qd = gluNewQuadric(); // Инициализируем указатель
		gluQuadricTexture(qd, 1); // Генерируем текстурные координаты
		glRotatef((GLfloat)timer, 0.0, 1.0, 0.0); // Поворот относительно нуля координат
		glTranslatef(pos.getX(), pos.getY(), pos.getZ()); // Двигаем в новую позицию
		glRotatef(90, 1, 0, 0); // Поворот на 90 по x
		gluSphere(qd, 3.0 * size, 30 * size, 30 * size); // Рисуем сферу
		
		remove();
	}

	// Обновление угла
	void update() {
		if (timer > 360) {
			timer = 0;
		}
		timer += speed;
	}
};

// Солнечная система
class SolarSystem {
private:
	Planet sun; // Солнце
	List<Planet> planets; // Список планет

public:

	// Геттеры
	const Planet& getSun() const {
		return sun;
	}
	const List<Planet>& getPlanets() const {
		return planets;
	}

	// Конструктор
	SolarSystem() {
		sun = Planet("textures/sun.bmp", 256, 256, 0, 13, Point3D(0, 0, 0));
		planets.push_back(Planet("textures/mercury.bmp", 256, 256, 1, 1.2, { -50.0f, 0.0f, 0.0f }));
		planets.push_back(Planet("textures/venus.bmp", 256, 256, 0.8, 1.8, {-70.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/earth.bmp", 256, 256, 0.7, 2, {-95.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/mars.bmp", 256, 256, 0.6, 1.7, {-115.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/jupiter.bmp", 256, 256, 0.3, 4, {-150.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/saturn.bmp", 256, 256, 0.25, 3.5, {-185.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/uranus.bmp", 256, 256, 0.2, 2.8, {-215.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/neptune.bmp", 256, 256, 0.1, 2.7, {-245.0f, 0.0f, 0.0f}));
		planets.push_back(Planet("textures/pluto.bmp", 256, 256, 0.08, 0.6, {-265.0f, 0.0f, 0.0f}));
	}

	// Отрисовка солнца и планет
	void draw() {
		sun.draw();
		for (auto tmp = planets.getHead(); tmp; tmp = tmp->getNext()) {
			tmp->Info().draw();
		}
	}

	// Обновление планет
	void update() {
		for (auto tmp = planets.getHead(); tmp; tmp = tmp->getNext()) {
			tmp->Info().update();
		}
	}
};

// Создаем объектную модель солнечной системы
SolarSystem solar;

// Тест классов
// Тесты для класса Point3D
void test_Point3D() {
	// Тест конструктора по умолчанию
	Point3D p1;
	assert(p1.getX() == 0);
	assert(p1.getY() == 0);
	assert(p1.getZ() == 0);

	// Тест конструктора с параметрами
	Point3D p2(1.0, 2.0, 3.0);
	assert(p2.getX() == 1.0);
	assert(p2.getY() == 2.0);
	assert(p2.getZ() == 3.0);

	// Тест сеттеров
	p2.setX(10.0);
	p2.setY(20.0);
	p2.setZ(30.0);
	assert(p2.getX() == 10.0);
	assert(p2.getY() == 20.0);
	assert(p2.getZ() == 30.0);

	cout << "Point3D tests passed!" << endl;
}

// Тесты для класса List
void test_List() {
	// Тест конструктора по умолчанию
	List<int> list;
	assert(list.getSize() == 0);
	assert(list.getHead() == nullptr);

	// Тест добавления элемента
	list.push_back(10);
	assert(list.getSize() == 1);
	assert(list.getHead()->Info() == 10);

	// Тест добавления нескольких элементов
	list.push_back(20);
	list.push_back(30);
	assert(list.getSize() == 3);
	assert(list.getHead()->Info() == 30); // последний добавленный элемент
	assert(list.getHead()->getNext()->Info() == 20); // предыдущий элемент
	assert(list.getHead()->getNext()->getNext()->Info() == 10); // первый добавленный элемент

	// Тест очистки списка
	list.clear();
	assert(list.getSize() == 0);
	assert(list.getHead() == nullptr);

	cout << "List tests passed!" << endl;
}

// Тесты для класса Planet
void test_Planet() {
	// Тест конструктора по умолчанию
	Planet p1;
	assert(p1.getSpeed() == 0);
	assert(p1.getTimer() == 0);
	assert(p1.getSize() == 0);
	assert(p1.getPos().getX() == 0);
	assert(p1.getPos().getY() == 0);
	assert(p1.getPos().getZ() == 0);

	// Тест конструктора с параметрами
	Planet p2("textures/earth.bmp", 256, 256, 0.7f, 2.0f, Point3D(-95.0f, 0.0f, 0.0f));
	assert(p2.getSpeed() == 0.7f);
	assert(p2.getSize() == 2.0f);
	assert(p2.getPos().getX() == -95.0f);
	assert(p2.getPos().getY() == 0.0f);
	assert(p2.getPos().getZ() == 0.0f);

	// Тест обновления таймера
	p2.update();
	assert(p2.getTimer() == 0.7f);
	p2.update();
	assert(p2.getTimer() == 1.4f);

	// Тест переполнения таймера (обновление после 360 градусов)
	for (int i = 0; i < 600; i++) {
		p2.update();
	}
	assert(p2.getTimer() < 360);

	cout << "Planet tests passed!" << endl;
}

// Тесты для класса SolarSystem
void test_SolarSystem() {
	// Тест конструктора
	SolarSystem solarSystem;
	assert(solarSystem.getSun().getSize() == 13.0f);
	assert(solarSystem.getPlanets().getSize() == 9); // 9 планет (включая Плутон)

	// Тест наличия планет
	assert(solarSystem.getPlanets().getHead()->Info().getPos().getX() == -265.0f); // Плутон
	assert(solarSystem.getPlanets().getHead()->getNext()->Info().getPos().getX() == -245.0f); // Нептун
	assert(solarSystem.getPlanets().getHead()->getNext()->getNext()->Info().getPos().getX() == -215.0f); // Уран

	// Тест обновления всех планет
	float initialTimer = solarSystem.getPlanets().getHead()->Info().getTimer();
	solarSystem.update();
	assert(solarSystem.getPlanets().getHead()->Info().getTimer() > initialTimer);

	// Тест изменения таймера для всех планет
	Node<Planet>* tmp = solarSystem.getPlanets().getHead();
	while (tmp) {
		float oldTimer = tmp->Info().getTimer();
		tmp->Info().update();
		assert(tmp->Info().getTimer() > oldTimer);
		tmp = tmp->getNext();
	}

	cout << "SolarSystem tests passed!" << endl;
}

// Отображение всей сцены
void display()
{
	glLoadIdentity();
	gluLookAt(X, Y, Z, X + X0, Y, Z + Z0, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Двигаем камеру
	if (moveAB)
	{
		X = X + moveAB * X0;
		Z = Z + moveAB * Z0;
	}
	if (moveLR)
	{
		angle += moveLR;
		X0 = sin(angle);
		Z0 = -cos(angle);
	}
	if (moveUp) {
		Y += moveUp;
	}

	// Отрисовка
	solar.draw();

	// Обновление таймеров
	if(anim)
		solar.update();
	glutSwapBuffers();
}

// Нажатие клавиш
void keyFunc(unsigned char key, int x, int y)
{
	// Выход Esc
	if (key == 27)
	{
		exit(EXIT_SUCCESS);
	}

	// Вкл / выкл анимацию
	if (key == 32) {
		anim = !anim;
	}
}

// Если нажали спец. клавишу
void keyPressed(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		moveLR = -0.01f;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		moveLR = 0.01f;
	}
	else if (key == GLUT_KEY_UP)
	{
		moveAB = 5.0f;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		moveAB = -5.0f;
	}
	else if (key == GLUT_KEY_PAGE_UP)
	{
		moveUp = 5.0f;
	}
	else if (key == GLUT_KEY_PAGE_DOWN)
	{
		moveUp = -5.0f;
	}
}

// Если отпустили спец. клавишу
void keyReleased(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		if (moveLR < 0)
			moveLR = 0;
		break;
	case GLUT_KEY_RIGHT:
		if (moveLR > 0)
			moveLR = 0;
		break;
	case GLUT_KEY_UP:
		if (moveAB > 0)
			moveAB = 0;

		break;
	case GLUT_KEY_DOWN:
		if (moveAB < 0)
			moveAB = 0;
		break;
	case GLUT_KEY_PAGE_UP:
		if (moveUp > 0)
			moveUp = 0;

		break;
	case GLUT_KEY_PAGE_DOWN:
		if (moveUp < 0)
			moveUp = 0;
		break;
	}
}

// Перерисовка экрана
void redisplay(int value)
{
	glutPostRedisplay();
	glutTimerFunc(10, redisplay, 0);
}

// Начальная инициализация
void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
}

// Проекция
void reshape(int width, int height)
{
	float ratio = ((float)width) / ((float)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, width, height);
}

// Главная функция
int main(int argc, char** argv)
{
	test_Point3D();
	test_List();
	test_Planet();
	test_SolarSystem();
	cout << "All tests passed successfully!" << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 150);
	glutCreateWindow("3D Solar System");
	glutIgnoreKeyRepeat(1);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyFunc);
	glutSpecialFunc(keyPressed);
	glutSpecialUpFunc(keyReleased);
	glutReshapeFunc(reshape);
	init();
	glutTimerFunc(0, redisplay, 0);
	glutMainLoop();
	return(0);
}