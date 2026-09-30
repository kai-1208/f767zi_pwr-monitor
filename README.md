# f767zi_pwr-monitor
## 概要
このプロジェクトは、f767ziボードでuartで電源の入力値を監視するものになります。

## 制御について
### 通信
このプロジェクトでは、usart6を使用しており、pg9をrx, pg14をtxに設定しております。

### cubemxの設定
- Pinout ViewでPG9をUSART6_RX, PG14をUSART6_TXに設定
- ConnectivityのUSART6で、ModeをAsynchronous, Parameter SettingsのBaud Rateを9600bpsに設定
- Clock ConfigurationでHLCKを216MHzに設定して、Reslove Clock Issues
