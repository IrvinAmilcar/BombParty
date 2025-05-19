# Magic Party💣💣💣
Magic Party é um jogo multijogador local com uma gameplay simples, divertida e desafiadora. O objetivo é escrever, o mais rápido possível, palavras que contenham a sílaba sorteada. Quem errar ou demorar demais...  — e a bomba estourou!

# Conceito 🧠
- A cada rodada, uma sílaba aleatória é exibida na tela.
- Os jogadores devem digitar uma palavra válida que contenha essa sílaba.
- Palavras repetidas não são permitidas.
- Quem não responder a tempo e a bomba estourar perde uma vida.
- Ao perder todas as vidas, o jogador é eliminado.
- O último jogador restante vence!

# Tecnologias utilizadas 🛠️
- Linguagem: C
- Biblioteca gráfica: Raylib – para interface, controle de entrada e renderização 2D

## Plataformas Suportadas
* Windows
* Linux
* MacOS

# VSCode Users (todas as plataformas)
*Note* Voce tem que ter um compiler toolchain instalado em adição ao vscode.

* Download do zip do jogo
* Abra a pasta no VSCode
* Run da build task ( CTRL+SHIFT+B or F5 )
* Pronto!

# Windows Users
Existem duas compiler toolchains disponiveis para windows, MinGW-W64(um compilador de graça usando GCC), e Microsoft Visual Studio
## Usando MinGW-W64
* Clique duplo no arquivo `build-MinGW-W64.bat` 
* CD para a pasta no seu terminal
* run `make`
* Pronto!

### Nota em MinGW-64 versions
Certifique-se de ter uma versão moderna do MinGW-W64 (não o mingw).
O melhor lugar para obtê-lo é no W64devkit em
https://github.com/skeeto/w64devkit/releases
ou na versão instalada com o instalador do raylib.
#### Se você instalou o raylib a partir do instalador,
Certifique-se de ter adicionado o caminho

`C:\raylib\w64devkit\bin`

Para a variável de ambiente path, para que o compilador que veio com o raylib possa ser encontrado.

NÃO INSTALE OUTRO MinGW-W64 de outra fonte, como o msys2, pois você não precisa dele.

## Microsoft Visual Studio
* Execute `build-VisualStudio2022.bat`
* Clique duas vezes no arquivo `.sln` gerado
* Desenvolva seu jogo
*Pronto!

# Usuários Linux
* CD na pasta de compilação
* execute `./premake5 gmake2`
* CD de volta à raiz
* execute `fazer`
* você está pronto para ir

# Usuários MacOS
* CD na pasta de compilação
* execute `./premake5.osx gmake2`
* CD de volta à raiz
* execute `fazer`
* você está pronto para ir