# Maré de Cobre

Protótipo de plataforma para Mega Drive, em C com SGDK. A fase tem 1.280 pixels de largura, cinco alvos e uma faixa amarela que marca a chegada. O cenário e o personagem foram criados para o protótipo. As ROMs de referência ficam fora do build.

## Jogar

Abra `out/rom.bin` em um emulador de Mega Drive. O jogo começa na fase.

| Controle do Mega Drive | Ação |
| --- | --- |
| Direcional esquerdo ou direito | Andar e mudar de direção |
| B | Saltar. Segure para ganhar altura. |
| A ou C | Atirar. Segure para repetir o disparo. |
| Start | Reiniciar a fase |

Uma queda reinicia a partida. Os cinco alvos são opcionais e a faixa amarela sinaliza a chegada.

## Compilar no Windows

Use o SGDK 2.11 e Java no `PATH`. Extraia o SDK para um caminho sem espaços e rode o comando na pasta `prototipo-sgdk`. [SGDK 2.11](https://github.com/Stephane-D/SGDK/releases/tag/v2.11).

```powershell
./build.ps1 -Sgdk 'C:\sdk\sgdk'
```

O script também aceita o caminho pela variável `GDK`. O build gera `out/rom.bin` e `out/symbol.txt`. Os PNGs indexados já estão em `res`. Para regenerá-los a partir das formas e paletas originais, rode `py -3 tools/generate_art.py`.

## Estrutura

- `src/main.c` implementa controle, física em ponto fixo, projéteis, câmera e HUD.
- `src/level.c` define as plataformas usadas pela colisão e pelo desenho. A câmera carrega as colunas que entram na tela e o fundo se move a um quarto da velocidade do primeiro plano.
- `res/art.res` registra os tiles e sprites no compilador de recursos do SGDK.

A ROM1 serviu de referência para a caminhada de aproximadamente 1,47 pixel por quadro e para o salto parado de 55 pixels. No protótipo, o salto longo mediu 55 pixels e 42 quadros até o pouso. O salto solto cedo mediu 26 pixels no teste. A calibração é por quadro em NTSC.

No Genesis Plus GX, um percurso só com entradas de controle cruzou todos os vãos, acertou os cinco alvos e chegou à faixa final. Também foram verificados o tiro nos dois sentidos, a rolagem de volta, a queda e o reinício por Start. A compilação limpa produziu uma ROM de 128 KiB idêntica à entregue. Nesta instalação do Windows, o link do SDK exibiu uma mensagem interna de LTO marcada como ignorada e concluiu a geração da ROM.
