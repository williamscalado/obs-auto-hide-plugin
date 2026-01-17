# git remote add origin git@github.com:williamscalado/obs-auto-hide-plugin.git

Este é um plugin nativo para **OBS Studio**, desenvolvido em C++ com Qt6. Ele automatiza a visibilidade de fontes e cenas no OBS baseando-se no conteúdo projetado pelo software **Holyrics**.

O objetivo principal é melhorar a transmissão ao vivo ou gravação, escondendo automaticamente elementos visuais (como lower thirds, logomarcas ou câmeras) quando um **versículo bíblico** é exibido no telão, e restaurando-os quando o versículo sai de cena.

---

## 🚀 O que este app faz

O plugin atua como um **cliente HTTP** que monitora o servidor local do Holyrics. Ele:

1.  Consulta periodicamente o status da projeção.
2.  Analisa o HTML retornado para identificar se o conteúdo é uma Bíblia, uma Música ou outro tipo de slide.
3.  Interage diretamente com a API do OBS (`libobs`) para alterar a visibilidade de fontes específicas em uma cena monitorada.

### Principais Funcionalidades
-   **Monitoramento em Tempo Real:** Conexão via HTTP Polling configurável.
-   **Detecção Inteligente:** Diferencia versículos bíblicos de letras de música.
-   **Controle Granular:** Permite escolher exatamente quais fontes esconder (ex: esconder apenas a fonte "Logo" mas manter a "Câmera").
-   **Restauração de Estado:** Opcionalmente restaura a visibilidade das fontes para como estavam antes da automação.
-   **Debounce/Delay:** Configuração de tempo de espera para evitar "piscas" na tela em trocas rápidas de slide.
-   **Modo Música:** Opção para desativar a automação caso uma música seja detectada (evita esconder a câmera durante o louvor).

---

## ⚡ Quick Start (Uso)

### Pré-requisitos
-   OBS Studio (versão 28 ou superior recomendada).
-   Holyrics rodando na mesma rede (ou na mesma máquina).

### Instalação
1.  Baixe a última versão na aba [Releases](#).
2.  **Windows:** Execute o instalador ou extraia para a pasta de plugins do OBS (`C:\Program Files\obs-studio\obs-plugins\64bit`).
3.  **macOS:** Execute o script `install-macos.sh` ou copie o `.plugin` para `~/Library/Application Support/obs-studio/plugins`.

### Configuração
1.  Abra o OBS Studio.
2.  No menu superior, vá em **Docks (Docas)** > **Auto Hide Scenes**.
3.  Uma janela de configuração abrirá. Configure:
    -   **URL Base:** Endereço do Holyrics (padrão `http://localhost:9000`).
    -   **Cena Monitorada:** A cena onde estão as fontes que você quer controlar.
    -   **Fontes:** Marque as caixas das fontes que devem sumir ao aparecer um versículo.
4.  Clique em **Salvar**.

---

## 🛠️ Build e Deploy

O projeto utiliza **CMake** como sistema de build.

### Estrutura de Diretórios Relevante
```
├── src/                # Código fonte C++
├── scripts/            # Scripts de automação de build/install
├── CMakeLists.txt      # Configuração do CMake
└── data/               # Arquivos de tradução e recursos
```

### ⚙️ Configuração do Ambiente (Dev)

Como este projeto depende dos headers do OBS Studio, você precisa baixá-los para a pasta `deps/` antes de compilar.

Execute os comandos abaixo na raiz do projeto:

```bash
mkdir -p deps
cd deps

# Clonar repositório do OBS Studio (apenas headers são necessários)
git clone --depth 1 https://github.com/obsproject/obs-studio.git

# Clonar SIMDE (dependência do OBS)
git clone --depth 1 https://github.com/simd-everywhere/simde.git

cd ..
```

### 🍎 Build no macOS

**Pré-requisitos:**
-   Xcode Command Line Tools.
-   CMake.
-   Qt 6 (`brew install qt@6`).
-   OBS Studio (código fonte ou binários com headers).

**Comando:**
```bash
# Permissão de execução no script
chmod +x scripts/build-macos.sh

# Executar build
./scripts/build-macos.sh
```
*O artefato será gerado na pasta `build/`.*

### 🪟 Build no Windows

**Pré-requisitos:**
-   Visual Studio (com carga de trabalho C++).
-   CMake.
-   Qt 6.
-   OBS Studio Dependencies.

**Comando:**
```batch
cd scripts
build-windows.bat
```

---

## 📡 Integração e Endpoints

Este plugin não expõe uma API; ele consome a API/View do Holyrics. Abaixo detalhamos como essa comunicação é feita para fins de debug e entendimento.

### Endpoint Consumido (Holyrics)

| Método | Endpoint | Descrição |
| :--- | :--- | :--- |
| `GET` | `/view/text` | Retorna o HTML do slide atual exibido no Holyrics. |

### Lógica de Parsing
O plugin faz um GET neste endpoint e busca por padrões no HTML retornado:

1.  **Detecção de Bíblia:**
    -   Procura por classes CSS: `class="bible_slide"`
    -   Procura por tags específicas: `<desc>` contendo referências.
2.  **Detecção de Música (opcional):**
    -   Analisa se o slide contém metadados de música para a funcionalidade "Desativar em música".

**Exemplo de fluxo:**
1.  Plugin -> GET `http://localhost:9000/view/text`
2.  Holyrics -> Retorna HTML `... <div class="bible_slide">João 3:16...`
3.  Plugin -> Detecta "Bíblia" -> Chama `obs_source_set_enabled(source, false)` nas fontes configuradas.

---

## ⚙️ Funcionalidades e Regras de Negócio

### Regras de Ativação
-   **Prioridade:** A detecção manual ou override do usuário no OBS tem prioridade se a opção "Restaurar estado" estiver desativada.
-   **Delay de Ação:** Se configurado um delay de 500ms, o plugin espera o slide ficar estável por 500ms antes de esconder as fontes. Isso previne que a interface "pisque" se o operador do Holyrics passar slides muito rápido.

### Variáveis e Configuração (CMake)

Para compilar, você pode precisar definir caminhos específicos caso suas bibliotecas não estejam nos locais padrão.

| Variável CMake | Descrição |
| :--- | :--- |
| `CMAKE_PREFIX_PATH` | Caminhos para Qt6 e LibOBS (Ex: `/opt/homebrew/opt/qt6`). |
| `CMAKE_BUILD_TYPE` | `Debug` ou `Release`. |

---

## 🔍 Logging e Trace

O plugin utiliza o sistema de log nativo do OBS Studio.

-   **Nível de Log:** Acompanha a configuração global do OBS.
-   **Localização dos Logs:**
    -   No OBS: Menu **Ajuda** > **Arquivos de Log** > **Ver arquivo de log atual**.
    -   Procure por entradas taggeadas com `[auto-hide-scenes]`.

**Exemplo de Log:**
```text
[auto-hide-scenes] Plugin carregado com sucesso.
[auto-hide-scenes] Conectado ao Holyrics em http://localhost:9000.
[auto-hide-scenes] Versículo detectado. Escondendo fonte: 'Logo Principal'.
```

---

## 🧪 Testes

### Teste de Conexão
Na interface de configuração do plugin, existe um botão **"Testar Conexão"**.
-   Ele realiza uma requisição imediata ao Holyrics.
-   Retorna sucesso se o servidor responder (HTTP 200).
-   Indica visualmente se o slide atual é reconhecido como um versículo ou não.

---

## 📄 Licença

Este projeto é distribuído sob a licença especificada no arquivo `LICENSE`.
