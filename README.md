# git remote add origin git@github.com:williamscalado/obs-auto-hide-plugin.git

Este é um plugin nativo para **OBS Studio**, desenvolvido em C++ com Qt6. Ele automatiza a visibilidade de fontes e cenas no OBS baseando-se no conteúdo projetado pelos softwares **Holyrics** ou **ProPresent**.

O objetivo principal é melhorar a transmissão ao vivo ou gravação, escondendo automaticamente elementos visuais (como lower thirds, logomarcas ou câmeras) quando um **versículo bíblico ou slide de apresentação** é exibido no telão, e restaurando-os quando o item sai de cena.

---

## 🚀 O que este app faz

O plugin atua como um **cliente HTTP** que monitora o servidor local ou remoto do Holyrics ou ProPresent. Ele:

1.  Consulta periodicamente o status da projeção via API.
2.  Analisa o payload retornado para identificar se há conteúdo sendo exibido.
3.  Interage diretamente com a API do OBS (`libobs`) para alterar a visibilidade de fontes específicas em uma cena monitorada.

### Principais Funcionalidades
-   **Monitoramento em Tempo Real:** Conexão via HTTP Polling configurável.
-   **Multi-Client:** Suporte nativo às APIs de apresentação do **Holyrics** e **ProPresent**.
-   **Transição Automática (Studio Mode):** Aciona autonomamente o botão de "Transição" caso o Modo Estúdio do OBS esteja aberto, evitando cortes secos (*Fade* orgânico).
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
2.  No menu superior, vá em **Docks (Docas)** > **Auto Hide Scenes** e clique no ícone de "Configurações" ⚙️ no painel.
3.  Uma janela de configuração abrirá. Na aba **Conexão**, configure:
    -   **Cliente:** Escolha entre Holyrics e ProPresent.
    -   **URL Base:** Endereço do software (padrão `http://localhost:9000` para Holyrics, ou `http://localhost:5050` padrão para servidor customizado).
4.  Na aba **Cenas**, configure:
    -   **Cena para Monitorar:** A cena onde estão as fontes que você quer controlar.
    -   **Fontes:** Marque as caixas das fontes que devem sumir quando o texto for projetado.
5.  Na aba **Comportamento**, ative opcionalmente a *Restauração de Estado*, *Pausar em Música*, e o *Acionar Transição Automática (Modo Estúdio)*.
6.  Clique em **Salvar**.

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

Este plugin não expõe uma API; ele consome a API de softwares. Abaixo detalhamos os endpoints usados para comunicação:

### Endpoints Consumidos

| Software | Método | Endpoint | Resposta (Expectativa) |
| :--- | :--- | :--- | :--- |
| **Holyrics** | `GET` | `/view/text` | HTML contendo classes `bible_slide` ou descrições no Node `<desc>`. |
| **ProPresent** | `GET` | `/v1/presentation/active` | Objeto `JSON` possuindo campo `presentation` root level preenchido. |

### Lógica de Parsing

-   **Holyrics**: Busca por `<div class="bible_slide">` ou `<desc>` no corpo HTML. O delay evita capturas temporárias acidentais do operador.
-   **ProPresent**: Interpreta a árvore do JSON para o slide corrente em modo Presentation e verifica nulidade do campo `presentation.id`. Se o usuário "limpar tela" na igreja, esse valor fica nulo e o plugin retorna o layout original no OBS.

**Exemplo de fluxo em Studio Mode:**
1.  Plugin -> GET API de Presentação / Holyrics
2.  Cliente -> Retorna carga útil dizendo "Temos um Slide ao Vivo!"
3.  Plugin -> Oculta localmente a "Logo Principal" na cena **PREVIEW**.
4.  Plugin -> Chama função `obs_frontend_preview_program_trigger_transition()` executando o fade da live automaticamente.

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
