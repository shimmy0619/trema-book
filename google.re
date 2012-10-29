= Google のデータセンター間接続

//lead{
データセンターを一台の巨大なコンピュータと見立てると、この巨大コンピュータ同士をつなぐネットワークが必要になります。常に時代の先端を行く Google は、OpenFlow を使って低コストにこのネットワークを作り上げてしまいました。
//}

== Google は OpenFlow をすでに導入している

2012 年 4 月、SDN や OpenFlow をテーマにした一大カンファレンスである Open Networking Summit において、Google の上級副社長兼フェローであるウルス・ヘルツル氏が衝撃の発表を行いました。Google はすでに OpenFlow を実戦投入しており、Google 内部のネットワークを OpenFlow で構築したというのです。インターネット界の巨人 Google は、OpenFlow のどのような特徴に着目し、どのように活用しているのでしょうか？ヘルツル氏の講演内容から、読み解いていきましょう。

Google が多くの巨大データセンターを保有し、インターネット上のさまざまな情報を収集していることは、みなさんもご存じでしょう。例えば Google が公開している資料からは、以下のようなすさまじいデータを発見できます。

 * 毎分 60 時間分の動画が YouTube にアップロード
 * 一日に処理するデータ量は数十ペタバイト以上
 * 毎月数十億ページを収集している

このように大量のデータを効率的に処理するために、Google はスループットの高いデータセンターを低コストに実現していることを知っている人も多いでしょう。

データセンター同士をつなぐネットワークに関してはどうでしょうか？Google は既存のネットワーク技術には満足していないようです。特にデータセンター間をつなぐ広域通信網 (WAN) 技術に関しては、OpenFlow を用いた新しい試みを始めています。なぜ彼らは既存の技術に満足しなかったのでしょうか？ポイントは、そのコストにあるようです。

=== 低コストな WAN への課題

高品質なサービスをどうやって低コストに提供するかは、すべてのサービス事業者にとって共通の悩みです。特に Google のような巨大な設備を持つサービス事業者では、小さなコスト削減の積み重ねが、結果として大きなコスト削減につながります。

コスト削減を考える際に大きな課題となるのが、WAN 回線の効率化です。Google は世界各国に巨大なデータセンターを多数持っており、これらの間をつなぐために専用の WAN 回線を引いています。これらの回線は容量が大きなものほど高価になるので、必要最小限の回線を最大限使いきる必要があります。

#@warn(具体的な回線コストをどこかからひっぱってきて載せてください)

しかし、WAN 回線の効率化は容易ではありません。従来のインターネットのルーティングでは、宛先に対して最短となるパスを選んで転送を行います。どれくらい帯域を使うかに関わらず最短となる経路を選択してしまうため、トラフィックが集中する回線や、逆にまったく使われない回線が出てきます。これは非常に大きな無駄で、コスト削減の余地があります。

=== Google の WAN 構成

Google の WAN は役割に応じて、以下の二つに分かれています。

 * I-Scale：インターネットに直接つながるネットワーク。インターネットを介して Google を利用するユーザ向けのトラフィックを運ぶ。
 * G-Scale：データセンター間をつなぐバックボーンネットワーク。内部向けトラフィックを扱う。

Google は、このうち G-Scale に OpenFlow を適用しました。以下の理由により、効率化の余地が大きいと判断したためです。

 * G-Scale はインターネットにつながらない閉じたネットワークなので、どんなトラフィックが流れるかを予測しやすい
 * G-Scale は Google 専用のネットワークなので、トラフィックごとの優先度を自由に決定できる

#@warn(この節は箇条書きでいきなり終わってしまうので、次の節へのつなぎとなる文がここに入るとよい)

== どのように WAN 回線を効率的に使う?

それでは、データセンター間を結ぶ WAN 回線を効率的に使うための課題を見ていきましょう。

まずは、@<img>{wan} のデータセンター A からデータセンター B へどの程度のトラフィックを流せるかを考えてみましょう。通常のルーティングでは、最短経路となるリンク 1 しか使用しないため、このリンクの帯域が上限となります。リンク帯域の上限を超えるトラフィックを扱うためには、リンク帯域の増設が必要になり、コストが増えてしまいます。

#@warn(図が全体に青系統なので、ぱっと見どれとどれが同じ部品か、という情報が分かりづらくなっています。PDF で読む人向けに色をつけてしまいましょう。2 章のユースケースの図と同じ色にしてください)

//image[wan][データセンター A からデータセンター B へどのくらいトラフィックを流せる？][scale=0.6]

しかし、@<chap>{openflow_usecases} で紹介したように、スイッチ C を経由する経路もあわせて使うことで、さらにトラフィックを流せるようになります (@<img>{multipath})。

//image[multipath][複数の経路を使ってトラフィックを転送する][scale=0.6]

このように複数経路を使うと、どの程度までトラフィックを流すことができるでしょうか？もし他のトラフィックが全くなければ、それぞれのリンク帯域の上限まで流せます。例えば、リンク 1 には、リンク帯域の 10 Gbps までトラフィックを流せます。複数のリンクを経由する場合には、流すトラフィック量を帯域が最小のリンクに合わせる必要があります。つまり、スイッチ C を経由する経路で流せるトラフィック量は、リンク 3 の帯域である 6Gbps までとなります。結局、@<img>{multipath} のネットワークでは、データセンター A からデータセンター B へ、最大で 16Gbps のトラフィックを流せるということになります。

しかし、実際に複数の経路を使ってトラフィックを流すには、次の 3 つの課題をクリアしなければなりません。

 1. 空いた帯域を調べるには？
 2. 空いた帯域をどう分け合うか？
 3. 複数の経路にどう振り分ける？

//noindent
それでは、それぞれの課題を詳しく見ていきましょう。

=== 課題 1：空いた帯域を調べるには？

一つめは、どうやって空き帯域を調べるかという問題です。@<img>{multipath} では、データセンター A からデータセンター B へは 16Gbps までトラフィックを流せると説明しました。しかし、普通は他のデータセンター間にもトラフィックがあるので、16Gbps のうち空いている帯域しか使えません。そこで実際にどの程度トラフィックを流せるかを知るためには、各リンクの空き帯域の情報が必要です。

しかし、離れた場所にあるリンクの空き帯域は普通の方法では調べられません。データセンター A は、直接つながるスイッチ A がやり取りしているトラフィック量を調べることで、リンク 1 とリンク 2 の空き帯域が分かります。しかし、データセンター A から離れたリンク 3 の空き帯域については、別の方法で調べなければなりません。もちろん実際のネットワークにはもっとたくさんのリンクがあるため、すべてのリンクの空き帯域を調べる必要があります。

=== 課題 2：空いた帯域をどう分け合うか？

二つめは、空いた帯域をデータセンター間でどう分け合うかという問題です。例えば、@<img>{conflict} のネットワークにおいて、リンク 1 の空き帯域が 10Gbps であったとします。もしデータセンター A とデータセンター B がそれぞれが勝手に、データセンター C へ 10Gbps のトラフィックを流そうとすると、当然このうち半分のトラフィックしかリンク 1 を通過できません。しかも通過できないトラフィックは、スイッチが破棄してしまいます。

これを避けるためにはトラフィック間での調停者が必要です。つまり、どちらか片方のトラフィックがリンク 1 の空き帯域を占有する、もしくはリンク 1 の空き帯域を仲良く分けあう、というように上から指示する仕組みが必要です。

//image[conflict][どのトラフィックが空き帯域を使うかを、うまく調停する必要がある][scale=0.6]

=== 課題 3：複数の経路にどう振り分ける？

三つめは、ひとつのデータセンターから出るトラフィックを複数の経路へどう振り分けるかという問題です。例えば、@<img>{traffic} 中のデータセンターからのトラフィックのために、リンク 1 経由の経路で 10Gbps、リンク 2 経由の経路で 6Gbps の帯域がそれぞれ利用できるとします。スイッチは、これらの帯域を超えないように、これらの経路に各サービスのトラフィックを振り分ける必要があります。

このためには、サービスごとのトラフィック量を知っている必要があります。なぜならば、OpenFlow スイッチが認識できるのはヘッダの情報だけで、サービスレベルの情報までは知らないからです。

//image[traffic][サービスごとのトラフィックを、それぞれどの経路に流すかを決定する必要がある][scale=0.6]

== G-Scale での WAN 回線の効率利用

ここでは Google のデータセンター間ネットワークである G-Scale が、前節で述べた課題をどのように解決して、WAN の効率化を実現しているかを見ていきましょう。

=== トラフィックエンジニアリングサーバの導入

前節の課題を解決するために、G-Scale では、ネットワーク全体の空き帯域の一括して管理するためのトラフィックエンジニアリングサーバ (TE サーバ) を導入しています(@<img>{te_server})。TE サーバは、各リンクの空き帯域の情報を集めます。そして、各データセンター間のトラフィックに対して、空き帯域を割り当てます。

//image[te_server][TE サーバは、空き帯域を把握し、各トラフィックへ割り当てる][scale=0.5]

リンクの空き帯域は、リンクの帯域とそのリンクに流れているトラフィック量を取得すれば、その差から分かります。一般的なスイッチは、これらに関する情報を収集し、保持しています。そのため、これらの情報をリンクに接続するスイッチから取得することは難しくありません。

このように、各リンクの空き帯域を把握し、その空き帯域をトラフィックに割り当てるという役目を、TE サーバが一括して行うことで、前節に上げた課題 1, 2 を解決しています。

=== WAN 接続部の構成

次に、@<img>{te_server} にも出てきた WAN 接続部の構成について見ていきます (@<img>{wan_equip})。WAN 接続部は、複数の OpenFlow スイッチと、それらを制御する OpenFlow コントローラと BGP 処理部から構成されています。

//image[wan_equip][それぞれの WAN 接続部毎に、OpenFlow コントローラと BGP 処理部が配置されている][scale=0.5]

WAN 接続部には、大量のトラフィックを扱えるように、10Gbps Ethernet のインターフェイスを 100 ポート以上持つ大規模な OpenFlow スイッチが複数台配置されています。この OpenFlow スイッチは、Google が自ら開発したものです。このような仕様を持つスイッチは市場には存在しないため、Google は自作を行ったようです。自前での装置開発にはそれなりのコストが掛かりますが、WAN の効率化により削減できるコストの方が大きいと、Google は判断したのでしょう。

BGP 処理部は、他の WAN 接続部の BGP 処理部との間で、データセンターから受け取った経路情報を交換します。この BGP 処理部には、オープンソースの経路制御ソフトウェアである Quagga を用いられています。

これらの OpenFlow スイッチを、OpenFlow コントローラが制御します。各 OpenFlow コントローラは、それぞれの WAN 接続部内の OpenFlow スイッチの制御を行なっているだけです。しかし、BGP や TE サーバからの情報を元にフローを決定することで、G-Scale 全体として WAN の帯域の有効活用を実現しています。

ネットワーク中の全 OpenFlow スイッチを一台のコントローラで制御するルーティングスイッチ (@<chap>{routing_switch}) とは異なり、G-Scale は複数の OpenFlow コントローラが協調してネットワークを制御しています。複数コントローラの協調動作については他にさまざまな方法が考えられるため、OpenFlow の仕様自体には規定されていません。G-Scale は、その協調動作の代表的な一例です。

===[column] どのように BGP は動作する？

データセンターが G-Scale からパケットを受け取るためには、データセンター内にあるホストの IP アドレスを、G-Scale に教えてあげる必要があります。そうしなければ、G-Scale は受け取ったパケットをどのデータセンターに送り届ければいいかを判断できません。

G-Scale では、この目的のために BGP を用いています。BGP は、異なる組織間で、アドレスに関する情報 (経路情報と呼びます) を交換するためのプロコトルです。インターネットのように、様々な組織から構成されるネットワークを作るためには欠かすことのできないプロトコルです。BGP では、それぞれの組織を AS と呼び、AS 単位での経路情報の交換を行います。

BGP は、通常異なる組織同士の接続に用いられますが、Google ではデータセンター間の接続に用いています。各データセンターと G-Scale それぞれを一つの組織に見立て、それぞれが一つの AS として動作しています。G-Scale は、BGP により各データセンターから経路情報をもらい、その情報を元にしてパケットの届け先を決定しています。

//image[bgp][BGP を用いてデータセンター A 内のアドレスを G-Scale に通知する][scale=0.5]

例えば、データセンター A 内のホストには 192.168.1.0/24 のアドレスを持っていたとします(@<img>{bgp})。データセンター A 内の BGP ルータはこのアドレスに対する経路情報を BGP を用いて、G-Scale の BGP 処理部に通知します。経路情報を受け取った BGP 処理部は、やはり BGP を用いて、G-Scale 内の他の BGP 処理部へと経路情報を転送します。このようにすることで、例えば宛先が 192.168.1.1 であるパケットを受け取った時に、そのパケットをデータセンター A へと送り届ければよいということを、G-Scale は知ることができます。

BGP には External BGP (E-BGP) と Internal BGP (I-BGP) の二種類の接続形態があります。E-BGP は異なる組織間の、I-BGP は組織内の情報交換に、それぞれ用います。@<img>{bgp} では、データセンター A の BGP ルータと G-Scale 内の BGP 処理部との間の接続が、E-BGP になります。また G-Scale 内の BGP 処理部同士は、I-BGP を用いて経路情報を交換しています。

===[/column]

== まとめ

Google が、WAN 回線の有効活用を行うために、データセンター間トラフィックの転送に OpenFlow をどのように活用しているかを紹介しました。

 * Google が OpenFlow を使う理由について説明しました。Google は WAN 回線を効率的に利用することで、低コストに多くのトラフィックを扱うために、OpenFlow を活用しています。
 * WAN 回線の効率化を実現する上での三つの課題を取り上げました。また、Google は、これらの課題を OpenFlow を用いて、どのように解決しているか紹介しました。

Google のような大規模事業者で実際に OpenFlow が動いているという事実は、これから OpenFlow に取り組もうとしている技術者にとって大きな励みとなるでしょう。Google とそのまま同じ事を実現するのはなかなか難しいですが、彼らのチャレンジからは多くのことが学べます。自身が取り組もうとしてる課題解決に、この章の内容が少しでも役に立てば幸いです。

== 参考文献

: ウルス・ヘルツル氏 Open Networking Summit 2012 での講演 ( @<tt>{http://youtu.be/VLHJUfgxEO4} )
  今回の取り上げたウルス・ヘルツル氏の講演内容が、Youtube に投稿されています。Google がどのようなチャレンジを行ったか、彼らの生の声を聞きたい人にとっては必見の動画です。

: Google を支える技術 (西田圭介著、技術評論社)
  この章ではネットワーク面でのコスト削減について取り上げましたが、この本ではデータセンター自体の運営コストなどについての分析が行われています。この本を読むことで、Google が如何にして何万台ものコンピュータを使ってサービスを実現しているかを学ぶことができます。

: インターネットルーティングアーキテクチャ - BGPリソースの詳解 (サム・ハラビ、ダニー・マクファーソン著、ソフトバンククリエイティブ)
  この章でも簡単に紹介しましたが、より深く BGP について学びたい人は、この本を読むとよいでしょう。具体的な例が多く紹介されており、BGP の動作をイメージしながら理解することができます。

