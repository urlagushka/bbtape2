# bbtape
## Внешняя сортировка слиянием для устройства типа лента

### Содержимое ленты (T)
Требования к содержимому ленты:
* Тип должен быть копируемым, перемещаемым
* Тип должен поддерживать операции сравнения
* Тип должен поддерживаться nlohmann::json

### Алгоритм сортировки
#### 1. Разбиение файлов
Разбиение исходного файла на фрагменты размера ```M / sizeof(T)```, где M - размер ОЗУ в байтах.
Алгоритм разбиения на псевдокоде:
```
пока количество файлов не будет достигнуто:
      создать временный файл
      поместить временный файл в держатель

      если нельзя считать в ОЗУ:
            переход к следующей итерации

      запись в ОЗУ из исходного файла
      сортировка ОЗУ
      запись во временный файл из ОЗУ
```
Гарантируется четное количество файлов на выходе.

#### 2. Слияние
```
пока количество файлов в держателе больше 1:
      держатель = стратегия_сортировки(держатель, ...)

      изменение размера блока ОЗУ при условии
      изменение количества потоков при условии
```
На каждом шаге, кроме последнего, гарантируется четное количество файлов.

### Оперативная память
#### Разбиение на блоки
В входном конфигурационном файле указывается размер ОЗУ в байтах.
На этапе слияния стратегия распределяет ОЗУ по количеству потоков, происходит разбиение ОЗУ на блоки
размера ```ram_size / thread_amount```, где ```ram_size = M / sizeof(T)```.

#### Балансировка блоков
В рамках одной операции слияния выделенный блок разделяется на левую и правую часть (для первого и второго файла соответственно). Разделение происходит на основе размеров левой, правой лент.
Алгоритм разбиения:
```
Введем обзначения:
Л - размер левой ленты
П - размер правой ленты
Б - размер блока
С - место разбиения
----
если Л = 0, то С = 0, возврат
если П = 0, то С = Б, возврат

если Л + П <= Б, то С = Л, возврат

*** пропорциональное разбиение блока по размеру Л и П ***
учет минимального размера
возврат

```

### Стратегия слияния
```
пока остались файлы:
      если очередь заполнена:
            дождаться раньшей операции слияния
      создать задачу по слиянию
      поместить задачу в очередь
```

### Откуда появилась многопоточность?
Есть тип лента, лента представляет из себя физический объект с некими обозначениями.
Есть тип устройство, устройство для управления лентой: чтение, запись, прокрутка, сдвиг.
Одно устройство может в один момент обрабатывать одну операцию слияния, в то время как
большее количество устройств может обрабатывать большее операций слияния в один момент времени.

Количество устройств задается в конфигурационном файле.

### Примеры сортировок
Входной файл:
```
{
  "delay": {
    "on_read": 1,
    "on_write": 3,
    "on_roll": 10,
    "on_offset": 5
  },
  "physical_limit": {
    "ram": XXX,
    "conv": 1
  },
  "tape": [961, 720, 689, ..., 931, 435, 309]
}

delay - задержки на операции с лентой (миллисекунды)
ram - размер ОЗУ в байтах
conv - количество устройств
tape - исходная лента
```
Тестируемый тип - int32_t. Количество элементов в исходном файле - 1000.
При тестировании количество устройств (conv) увеличивалось от 1 до N.
Как можно заметить, в один момент время сортировки перестает уменьшаться (с ростом количества устройств, блок ОЗУ для слияния становится меньше)

#### ram: 256
![ram256](https://github.com/urlagushka/bbtape2/blob/main/pictures/ram256.png)
#### ram: 512
![ram512](https://github.com/urlagushka/bbtape2/blob/main/pictures/ram512.png)
#### ram: 1024
![ram1024](https://github.com/urlagushka/bbtape2/blob/main/pictures/ram1024.png)
#### ram: 2048
![ram2048](https://github.com/urlagushka/bbtape2/blob/main/pictures/ram2048.png)

### Сборка и запуск
#### MacOS (необходим gcc14)
```
-- если не установлен gcc14
brew install gcc

git clone https://github.com/urlagushka/bbtape2.git

cd bbtape2
mkdir build && cd build

cmake -DCMAKE_C_COMPILER=/opt/homebrew/Cellar/gcc/14.2.0_1/bin/gcc-14 \
      -DCMAKE_CXX_COMPILER=/opt/homebrew/Cellar/gcc/14.2.0_1/bin/g++-14 \
      -DCMAKE_BUILD_TYPE=Release \
      ..

make

-- запуск программы
./bbtape_example <src.json> <dst.json>

-- запуск тестов
ctest -V
```

#### Linux
```
git clone https://github.com/urlagushka/bbtape2.git

cd bbtape2
mkdir build && cd build

cmake ..

make

-- запуск программы
./bbtape_example <src.json> <dst.json>

-- запуск тестов
ctest -V
```