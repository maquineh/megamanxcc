# Análise das demos de Mega Man X para Mega Drive

As três ROMs de TiagoSC contêm o mesmo motor 68000 e os mesmos blocos de dados. Cada variante configura a fase inicial, um campo de estado e o tamanho da transferência de gráficos. Cada arquivo tem 1.685.868 bytes (`0x19B96C`). TiagoSC descreveu o projeto como escrito em assembly com o Macro Assembler AS e publicou as três variantes em julho de 2019. [Tópico do autor no SpritesMind](https://gendev.spritesmind.net/forum/viewtopic.php?p=33745).

## Fase de abertura e diferenças entre arquivos

| ROM | Fase no Mega Drive | ID no Mega Drive | ID documentado no SNES | Checksum do cartucho |
| --- | --- | --- | --- | --- |
| ROM1 | Central Highway | `00` | `00` | `D089` |
| ROM2 | Chill Penguin | `01` | `08` | `D4FA` |
| ROM3 | Armored Armadillo | `02` | `03` | `D71A` |

As três ROMs chegaram à fase indicada no teste de 180 quadros sem entrada, em 256 × 224, com o Genesis Plus GX v1.7.4 (`b9c2316`). O seletor de fase e o campo de estado também foram lidos na RAM do emulador. O teste cobriu a abertura das fases. Os IDs do SNES vêm do mapa de RAM publicado para `$7E:1F7A`. [Mapa de RAM do TASVideos](https://tasvideos.org/GameResources/SNES/MegaManX/RAMMap).

| Offset no arquivo | ROM1 | ROM2 | ROM3 | Efeito observado na desmontagem |
| --- | --- | --- | --- | --- |
| `0x00018E–0x00018F` | `D089` | `D4FA` | `D71A` | Checksum no cabeçalho. Os três valores batem com a soma de palavras a partir de `0x200`. |
| `0x01E789` | `00` | `00` | `0F` | Valor inicial escrito em RAM `$FFCB46` pela instrução em `$01E786`. O código lê e altera bits desse campo. |
| `0x01E7A3` | `00` | `01` | `02` | ID inicial escrito em RAM `$FFC904` pela instrução em `$01E7A0`. |
| `0x01E97A–0x01E97B` | `1640` | `1AB0` | `1CC0` | Argumento que controla a transferência dos gráficos da fase para a VRAM. |

ROM1 e ROM2 diferem em cinco bytes. ROM3 difere de cada uma das outras em seis. O padrão de testes de bits em `$FFCB46`, incluindo o teste em `$0045D4` e a alteração em `$00D4EE`, sugere uma máscara de habilidades ou melhorias. A correspondência individual dos bits ainda precisa ser confirmada.

## Caminho do reset ao carregamento da fase

Os endereços da ROM usam base zero. A RAM de trabalho é mostrada na faixa de endereços de 24 bits do 68000.

| Endereço | Função identificada |
| --- | --- |
| `$000000–$0000FF` | Vetores do 68000. Reset em `$000208`, interrupção horizontal em `$000FA0` e vertical em `$000BC8`. |
| `$000100–$0001FF` | Cabeçalho `SEGA MEGA DRIVE`, títulos `ROCKMAN X` e `MEGAMAN X`, região `JUE`, fim da ROM em `$19B96B`. |
| `$000208` | Inicialização de RAM e do VDP. |
| `$00077C` | Leitura do controle pelo registrador `$A10003`. |
| `$00049C` | Solicita acesso ao barramento do Z80 e copia o bloco de áudio a partir de `$0ADF00` para `$A00000`. |
| `$000B60`, `$01E764` | Inicializa os subsistemas e o estado da demo. O laço principal em `$000B9E` despacha estados por uma tabela em `$000BB4`. |
| `$01E842–$01E859` | Lê o ID em `$FFC904`, multiplica por quatro e salta pelo ponteiro correspondente na tabela em `$01E84E`. |
| `$01E960`, `$01F1AA` | O carregador lê o primeiro ponteiro do descritor da fase e chama a rotina de descompressão, com saída em RAM `$FF0000`. |
| `$01E886` | Transfere o bloco de gráficos pelo VDP para a VRAM. Nesse carregamento, o destino começa em `$0000`. |
| `$01EBBE`, `$01E9B4` | Usa `$FFC905` para selecionar um descritor adicional e carrega blocos de paleta e outros parâmetros da área. |

O carregador converte o argumento em grupos de 32 bytes. Assim, o primeiro bloco de gráficos enviado à VRAM tem 22.784 bytes na ROM1, 27.328 na ROM2 e 29.440 na ROM3. A seleção de fase e essa extensão de cópia são valores separados nas demos. [Manual técnico do Sega Genesis](https://nemesis.hacking-cult.org/MegaDrive/Documentation/GenesisSoftwareManual.pdf).

| Fase | Entrada na tabela | Descritor principal | Primeiro bloco comprimido |
| --- | --- | --- | --- |
| Central Highway | `$01EA2E` | `$02C108` | `$03307C` |
| Chill Penguin | `$01EA70` | `$02C18C` | `$03CB3C` |
| Armored Armadillo | `$01EACC` | `$02C23C` | `$0443A0` |

TiagoSC disse que escreveu as rotinas da demo, usou o mapa de RAM do SNES como referência para valores e manteve o formato de fases do original. Ele também citou o driver XGM na descrição de 2018. A análise local confirmou a carga de um bloco para o Z80. A versão exata do driver de 2019 e a compatibilidade integral dos formatos de fase seguem em aberto. [Descrição do autor](https://gendev.spritesmind.net/forum/viewtopic.php?p=33745).

## Comparação de Central Highway

O arquivo local `Mega Man X (USA) (Rev 1).sfc` tem 1.572.864 bytes, cabeçalho LoROM com revisão `01`, CRC32 `DED53C64` e SHA-256 `b8f70a6e7fb93819f79693578887e2c11e196bdf1ac6ddc7cb924b1ad0be2d32`. Os testes usaram Snes9x 1.63 (`fae2fea`) e Genesis Plus GX v1.7.4 (`b9c2316`). O SHA-256 coincide com o publicado para a revisão americana 1.1. [Identificação da revisão](https://github.com/mstan/MegaManXSNESRecomp).

| Trecho ou sistema | Resultado |
| --- | --- |
| Entrada na fase | O SNES passa pela tela de título e pelo início de um novo jogo. A ROM1 entra diretamente em Central Highway. |
| Movimento e X-Buster | As duas versões aceitam caminhada, salto, tiro simples e tiro carregado. Partindo de X=128, 30 quadros de direita deram X=167 no SNES e X=171 na ROM1. O salto parado com botão mantido subiu 55 px nas duas versões e retornou ao chão após 44 quadros no SNES e 43 na ROM1. |
| Percurso | A ROM1 tem rolagem, tráfego, buracos, patrulhas aéreas e inimigos blindados. No trecho com veículos perto de X=910, as duas execuções mostraram a mesma disposição geral de pista e veículos, com diferenças de paleta e enquadramento vertical. |
| Pausa | `Start` abriu o menu de armas e impediu o deslocamento no SNES. Na ROM1, X foi de 128 a 215 em 60 quadros de direita tanto com `Start` quanto no teste de controle. |
| Encontro final | A nave chegou nas duas versões e lançou veículos. Atirar repetidamente na nave pode derrubá-la e destruir a pista. Na ROM1, o trecho também pode terminar em morte e reinício da fase. |
| Desfecho canônico | No SNES, derrotar dois carros aciona Vile. A derrota roteirizada de X, a chegada de Zero e a abertura da seleção de fases completam a introdução. As execuções locais chegaram à nave. A sequência posterior do SNES foi conferida na documentação do replay da mesma revisão e no guia. [TAS de FractalFusion](https://tasvideos.org/1370S) · [Guia da fase](https://gamefaqs.gamespot.com/snes/588479-mega-man-x/faqs/67191). |

A principal lacuna na ROM1 está no caminho de conclusão. A tabela de objetos em `$0023F8` aponta a entidade da nave (`$46`) para `$0107E6`. Seu estado de voo usa a tabela em `$010936`. A rotina em `$010A7E` consulta `$FFCBAA`, um contador de objetos ativos do tipo `$45` no trecho, incrementado quando o objeto nasce e decrementado na remoção. Com o contador em `2`, ela volta ao subestado `4` de voo. A condição de dano em `$010922` leva à sequência de queda em `$010CEC`. A desmontagem desse fluxo revelou a repetição da batalha e a queda, sem uma transição para Vile no caminho mapeado.

Uma sondagem em RAM do Genesis Plus GX reproduziu a decisão da rotina. Com a nave no subestado de voo `8`, o contador em `0` permitiu criar um objeto `$45`. Com o contador em `2`, a nave voltou ao subestado `4`. As medições de movimento e pausa usaram apenas entradas de controle. Algumas sondagens do encontro final restauraram energia ou ajustaram temporariamente o estado de entidades no emulador para isolar ramificações. As ROMs permaneceram com os hashes registrados. A análise verificou a rota da nave, sem inventariar todos os objetos da demo ou descartar rotinas desconectadas em outras áreas do binário.

## Assembly ou SGDK

Com os arquivos disponíveis, a escolha para um projeto novo e recompilável é **SGDK com a lógica em C**. A pasta contém ROMs compiladas e o relatório. O SGDK oferece uma cadeia de compilação 68000, ferramentas de recursos e APIs de controle, mapas, sprites, DMA e sincronização com VBlank. Isso dá uma base de desenvolvimento sem depender de alterar offsets no cartucho. A ROM1 pode servir como referência de comportamento do hardware e das medições desta análise. Para um projeto novo, use conteúdo próprio ou licenciado. [Projeto SGDK](https://github.com/Stephane-D/SGDK) · [API de sincronização](https://github.com/Stephane-D/SGDK/blob/master/inc/sys.h) · [API de DMA](https://github.com/Stephane-D/SGDK/blob/master/inc/dma.h).

Se houver acesso ao fonte e aos recursos do TiagoSC com autorização para modificá-los, manter o assembly é a opção de menor retrabalho para continuar aquela base. O build original já inclui inicialização, interrupções horizontais e verticais, carregador de gráficos e driver de áudio. A primeira verificação seria remontar o fonte com Macro Assembler AS e confrontar o resultado com os binários de referência. Uma migração integral para SGDK teria de adaptar esses subsistemas e validar o comportamento de novo. [Descrição do autor](https://gendev.spritesmind.net/forum/viewtopic.php?p=33745).

A escolha entre C e assembly para rotinas críticas depende de medições de carga por quadro, transferências no VBlank e sprites por linha. As execuções desta análise mediram comportamento das ROMs existentes, de modo que ainda não há um benchmark comparável de uma implementação SGDK.

## Identificação dos arquivos

| ROM | SHA-256 |
| --- | --- |
| ROM1 | `becd3913fd22a56cb3087609a07b4104d8f17026833902ecde8010cd0cdea1e2` |
| ROM2 | `81ae5e2940ee25df6c81cc860d9ccfd6617988a7c186ae91d9cbd121b8ccafc2` |
| ROM3 | `5ac2c7c62dbd2a3e9b14b7534263efb65ece74a5b6aac37af5d8749c7d0341df` |

O REA inventariou o ZIP local de 2019 e confirmou os mesmos três SHA-256. Evidência `ev_e459e581f7f31a22150cb7b1f362f60026f78e5748ed7b756ccb5ec08d0e8719`. Como o REA recusou a imagem bruta de cartucho, a análise de código usou desmontagem 68000 com Capstone 5.0.7. Os MD5 locais também coincidem com os hashes publicados para as três demos. [Registro do RetroAchievements](https://retroachievements.org/forums/topic/13066).
