# Processador de imagens — Projeto 1

Programa em C99/SDL3 para carregar uma imagem, convertê-la para cinza, analisar
seu histograma e alternar a equalização. Baseado nos exemplos do professor André
Kishimoto.

## Integrantes

- Mateus Teles Magalhães — 10427410
- Gabriel Erick Mendes — 10420391
- Rafael Moutinho Tessarotto — 10395682

As funções de montagem e análise do histograma foram preparadas pelo Rafael. A adaptação das funções do professor Kishimoto foram feitas em conjunto por Gabriel e Mateus.

## Funcionalidades e controles

A imagem é carregada pelo caminho informado no terminal. O programa informa se
ela era colorida ou cinza e aplica automaticamente a fórmula
Y = 0.2125 R + 0.7154 G + 0.0721 B quando necessário.

| Controle | Resultado |
|---|---|
| Equalizar | Mostra a versão equalizada e atualiza histograma e informações |
| Ver original | Restaura a imagem original em cinza sem reler o arquivo |
| Resolução original | Muda a imagem e a janela para as dimensões do arquivo |
| 1024x768 | Retorna à resolução inicial |
| S, com uma das janelas em foco | Salva output_image.png na pasta atual do terminal |
| Fechar qualquer janela | Encerra o aplicativo e libera os recursos |

A janela principal inicia com 1024x768 pixels e centralizada no monitor principal.
Quando a imagem em resolução original excede a tela, a janela é posicionada no
canto superior esquerdo. O posicionamento considera as bordas da janela.

A janela secundária é filha da principal, tem tamanho fixo de 640x560. Mostra 256 barras, média, desvio padrão e classificações. Os botões mudam
de cor nos estados normal, mouse sobre o botão e pressionado. Soltar o mouse
fora do botão cancela a ação.

A exibição em 1024x768 preenche toda a janela e pode mudar a proporção aparente
da imagem. Os pixels originais guardados na memória não são redimensionados.
O PNG salvo corresponde ao conteúdo renderizado, no tamanho atual, incluindo
o fundo cinza em regiões transparentes. A gravação sobrescreve um arquivo
existente e informa no terminal se ele foi criado, sobrescrito ou se houve erro.

## Requisitos

- GCC com suporte a C99, Make e pkg-config.
- Bibliotecas de desenvolvimento SDL3, SDL3_image e SDL3_ttf.
- Ambiente gráfico: desktop Linux, WSL com WSLg ou Windows.

Ambiente utilizado: WSL Ubuntu 24.04.3, GCC 13.3.0, SDL3 3.4.16,
SDL3_image 3.4.6 e SDL3_ttf 3.2.2.

## Compilar e executar no Linux/WSL

Abra o terminal na pasta do projeto. No Ubuntu/WSL, caso as dependências
não estejam instaladas, execute:

```sh
bash scripts/install_dependencies.sh
```

O script requer internet e sudo para instalar as dependências.

Compile e execute:

```sh
make
./programa assets/kodim23.png
```

Para usar outra imagem, substitua `assets/kodim23.png` pelo caminho do arquivo.
Coloque o caminho entre aspas se contiver espaços.

## Compilar e executar no Windows

No terminal UCRT64 do MSYS2, instale as dependências:

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-sdl3-ttf
```

Na pasta do projeto, usando o mesmo terminal:

```sh
make
./programa.exe assets/kodim23.png
```

## Arquivos necessários

Mantenha a pasta `assets` ao lado do executável. A fonte DejaVu Sans está
incluída em `assets/fonts/DejaVuSans.ttf`, acompanhada de sua licença.
A licença do código-base está no arquivo `LICENSE`.
