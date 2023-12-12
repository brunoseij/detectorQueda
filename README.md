# Projeto Detector de Queda com NodeMCU v3 e Botão

## Descrição

Este é um projeto de um detector de queda que utiliza a placa NodeMCU v3, um sensor de impacto KY-031, um acelerômetro MPU 6050, sinalizações visuais por LED e sonoras por um buzzer. Adicionalmente, um botão foi integrado ao sistema para acionar o envio de notificações através de interrupção. O sistema é capaz de enviar notificações via requisições HTTP para o serviço IFTTT, permitindo integração com diversas plataformas.

## Componentes Utilizados

- NodeMCU v3
- Sensor de Impacto KY-031
- Acelerômetro MPU 6050
- LED
- Buzzer
- Botão

## Configuração do Projeto

1. **Instalação das Bibliotecas:**
   Certifique-se de instalar as bibliotecas necessárias para o NodeMCU v3 e o MPU 6050. Utilize o Gerenciador de Bibliotecas da Arduino IDE para isso.

2. **Configuração do IFTTT:**
   - Crie uma conta no [IFTTT](https://ifttt.com/).
   - Crie um novo applet com o "IF This" sendo uma Webhooks.
   - Anote a chave da Webhooks e o nome do applet, será utilizada no código do NodeMCU.

3. **Configuração do Código:**
   - Abra o arquivo `DetecQueda.ino`.
   - Insira sua rede Wi-Fi e senha (`NOME_REDE` e `SENHA_REDE`).
   - Substitua `SEU_TOKEN_IFTTT` pelo token obtido no passo anterior.
   - Substitua `NOME_APPLET` pelo nome dado ao applet criado.

4. **Conexão dos Componentes:**
   - Conecte o sensor de impacto KY-031, o acelerômetro MPU 6050 e o botão conforme o esquema de ligação abaixo.

## Uso

1. Carregue o código para a placa NodeMCU utilizando a Arduino IDE.
2. Abra o Monitor Serial para verificar mensagens de depuração.
3. O sistema iniciará e monitorará quedas ou impactos.
4. Pressione o botão para acionar o envio de notificações.
5. Em caso de detecção ou acionamento do botão, o NodeMCU enviará uma requisição HTTP para o IFTTT, que pode ser configurado para notificar por e-mail, mensagem de texto, entre outras opções.

## Observações

- Certifique-se de que todos os componentes estão conectados corretamente.
- Realize calibrações necessárias no MPU 6050 para garantir precisão nas medições.
