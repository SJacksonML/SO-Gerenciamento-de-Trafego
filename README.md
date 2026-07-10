# Gerenciamento de Tráfego

O Simulador de tráfego urbano é um projeto de sistema desenvolvido em C
utilizando threads, mutexes, semáforos e variáveis de condição para
demonstrar conceitos de Sistemas Operacionais como sincronização, exclusão
mútua, coordenação de threads e prevenção de deadlocks.

---

## Como executar

```bash
make          # compila para bin/simulador
make run      # compila (se preciso) e roda com os parâmetros padrão
make clean    # remove obj/ e bin/ gerados
```

### Requisitos

- `gcc` com suporte a `-std=gnu11`
- `pthread` (POSIX threads)
- Um terminal que interprete códigos ANSI (limpeza de tela), como qualquer
  terminal Linux/macOS ou o Windows Terminal / PowerShell 7+ no Windows

### Rodando no Windows

O código depende de APIs POSIX (`pthread.h`, `unistd.h`/`usleep`), que não
existem nativamente no Windows/MSVC. Os caminhos mais práticos, sem precisar
alterar o código-fonte:

- **WSL** (mais simples): `wsl --install`, depois `sudo apt install build-essential` e seguir os comandos acima normalmente.
- **MSYS2 + MinGW-w64**: o runtime "posix" do MinGW-w64 implementa
  `pthread.h`/`unistd.h`. No terminal **MSYS2 MinGW64**:
  `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make` e depois `make`.

Alternativamente, o código pode ser executado pelo codespace do github idependente do sistema operaacional do usuário.

---

## Estrutura do projeto

```simulador de trafego
.
├── 📁 include
│   ├── 📄 clock.h    # Definições do relógio lógico global (tick, mutex, cond)
│   ├── 📄 map.h      # Estrutura da malha urbana, células e tipos de tile
│   ├── 📄 render.h   # Funções de exibição e renderização da simulação em ASCII
│   ├── 📄 sync.h     # Sincronização: mutex por célula, semáforos de cruzamento, prioridade de ambulância
│   └── 📄 vehicle.h  # Modelagem, propriedades e ciclo de vida dos veículos
│
├── 📁 assets
│   └── 📄 map.txt    # Arquivo de configuração textual da matriz do mapa
│
└── 📁 src
    ├── 📄 clock.c    # Implementação do relógio lógico (broadcast de tick para as threads)
    ├── 📄 main.c     # Ponto de entrada do simulador e inicialização das threads
    ├── 📄 map.c      # Carregamento do mapa e validação de movimento por tipo de célula
    ├── 📄 render.c   # Atualização visual da tela e interface do terminal
    ├── 📄 sync.c     # Mutex de célula, semáforos de cruzamento e prioridade de ambulância
    └── 📄 vehicle.c  # Ciclo de vida do veículo e lógica de movimento (car_advance)
```

---

## Decisões de implementação

### Mapa

O mapa é carregado de um arquivo texto (`assets/map.txt`) para uma matriz de
células (`map.h`/`map.c`). Cada célula tem um tipo:

| Caractere | Tipo | Significado |
|:---:|---|---|
| `#` | `CELL_WALL` | parede / não é via |
| `+` | `CELL_INTERSECTION` | cruzamento, controlado por semáforo |
| `^` | `CELL_ONE_WAY_N` | via de mão única sentido Norte (linha diminui) |
| `v` | `CELL_ONE_WAY_S` | via de mão única sentido Sul (linha aumenta) |
| `>` | `CELL_ONE_WAY_E` | via de mão única sentido Leste (coluna aumenta) |
| `<` | `CELL_ONE_WAY_W` | via de mão única sentido Oeste (coluna diminui) |
| `C` | `CARRO` | Célula ocupada por um carro |
| `A` | `AMBULÂNCIA` | Célula ocupada por uma ambulância |
| `?` | `INDEFINIDO` | Célula ocupada por um veículo recém gerado ou com problema de atualização |

Mão dupla é representada por duas faixas de mão única em sentidos opostos
lado a lado (ex.: `^v` ou `<>`); não existe compartilhamento de célula entre
sentidos, cada faixa tem sua própria célula e seu próprio mutex.

Células identificadas por "?", quando ocorrem, acabam eventualmente retornando à lógica esperada de execução em poucos ticks

---

## Membros e responsabilidades

| Nome | Responsabilidades |
|------|-------------------|
| Aisha | Mapa e Renderização |
| Elilúcio | Veículos e Threads |
| Jackson | Sincronização |
| Ícaro | Relógio, Main e Integração |
