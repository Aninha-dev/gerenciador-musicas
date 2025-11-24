# Gerenciador de Músicas WAV em C

Este projeto é um **gerenciador de arquivos de áudio no formato WAV**, desenvolvido em C, que permite:

- Ler arquivos '.wav' de uma pasta escolhida pelo usuário  
- Extrair informações a partir do nome do arquivo (artista, nome da música, gênero)  
- Calcular a duração aproximada de cada música a partir do arquivo WAV  
- Organizar automaticamente as músicas em pastas por gênero  
- Criar playlists por gênero e tocar músicas usando o player padrão do Windows
- Salvar um índice das músicas em `musicas_index.txt` para facilitar consultas futuras 
- Gerar um relatório com estatísticas das músicas cadastradas  

---

## Formato esperado dos arquivos .wav

Os arquivos devem seguir o seguinte padrão de nome:
```text
Artista - Nome da Música - Genero.wav
