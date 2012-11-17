= データセンターを Trema で作る

//lead{
執筆中です。
//}

== Trema と OSS でデータセンターを作ろう

一台のコンピュータ上で、リソースの管理や基本サービスの提供に OS が必要になるように、データセンターにも同じ機能を提供するソフトウェア階層が必要です。IaaS はその一つで、ユーザやアプリケーションに対して仮想的なリソースの貸し借りを行います。たとえば「192.168.0.1/24 のネットワークでつながった高性能サーバ 16 台がほしい」というリクエストを受けると、IaaS はスペックの高い VM を物理マシン上に起動し、専用の仮想ネットワークを作って VM 同士をつなぎます。このように、データセンターのリソースを仮想化して使いやすく提供するのが IaaS です。

IaaS のネットワーク仮想化には OpenFlow が適しています。(理由をここに)。IaaS では、必要なとき仮想ネットワークを作り、使い終われば削除します。また IaaS では多くのユーザを収容しているため、仮想ネットワーク作成・削除は、他のユーザに影響を与えないように行う必要があります。

== Wakame-VDC

Trema を利用した本格的な IaaS プラットフォームが、Wakame-VDC です。すでに九州電力など多くの企業のプライベートクラウド構築基盤として実績がある上、なんとすべてがオープンソースとして公開されています。

Wakame-VDC は、IaaS 型のクラウド構築基盤ソフトウェアです。オープンソースライセンスである LGPL v3 に基づき公開されています。開発は任意団体である Wakame Software Foundation (WSF) が行っています。2009 年 4 月に株式会社あくしゅによって、Ruby で書かれた最初のコードがコミットされて以来、2012 年 9 月現在で計 22 企業・団体が所属し、今もなお積極的に開発が継続されています。Wakame-VDC も開発者を募集しております。気軽にパッチを送ってください。Github なら、Pull Request と言うスタイルで、それを支援してくれます。

Wakame-VDC の情報は次の URL から入手できます。

 * Wakame-VDC開発リポジトリ: @<href>{https://github.com/axsh/wakame-vdc}
 * Wiki ドキュメント: @<href>{http://wakame.jp/wiki/}

== エッジスイッチによるネットワーク仮想化

Wakame-VDC は、@<chap>{sliceable_switch} とは異なり、OpenFlow 未対応のスイッチで構成されたネットワークで動作するよう設計されています。IaaS を構成する物理サーバ内で動作するソフトウェアスイッチのみを OpenFlow を用いて制御することで、仮想ネットワークを実現します。

仮想ネットワークで必要な機能は、ユーザ毎のトラフィックの分離です。Wakame-VDC ではエッジのスイッチの制御のみでこれを実現しています。基本的なアイディアは、エッジスイッチを、物理ネットワークと、仮想ネットワークの境界線と考えて、エッジスイッチで相互に乗り入れるための変換ルールを用意しておいて、うまく物理と仮想の間を行き来できるようにしようと言うものです。

仮想ネットワークで使えるパケットを、うまく物理ネットワークで使えるように書き換えて、目的のNICへパケットを届けます。受け取った側では、物理ネットワークを旅してきたパケットを、再び仮想ネットワーク向けに書き換えるのです。仮想マシンは、仮想ネットワークだけ知っていれば良く、エッジスイッチはその仮想ネットワーク用のパケットを、うまく物理ネットワークに流して届くように変換する方法を知っていれば良いと言うことになります。

=== トラフィックの分離

一般的な Ethernet スイッチから構成されるネットワークでは、MAC アドレスを識別して、パケットの転送が行われます。そのため、きちんと届け先の VM の MAC アドレスを、宛先フィールドに入れてあげれば、ネットワークはそのパケットを宛先まで届けてくれます。

#@# ブロードキャストと区別するためにユニキャストであることが分かる表現とする必要がある（ただしユニキャストという用語は使わない）。そのため「届け先の MAC アドレスを宛先にいれる」と表現している。by kazuya

複数ユーザを収容する IaaS では、異なるユーザが同じ IP アドレスを持っている場合がありますが、それでも問題はありません。L2 ネットワークでは MAC アドレスを使って転送を行なうので、MAC アドレスがネットワーク全体でユニークに割り当てられていれば、問題はありません(@<img>{unicast})。

//image[unicast][一般的な Ethernet スイッチでのトラフィック分離]

それでは何か特別なことをしなくても、トラフィックの分離はできるのでしょうか？実は、@<chap>{router_part1} で説明した ARP の取り扱いに注意が必要になるのです。例えば、@<img>{arp_entangle} のようなネットワークを考えてみましょう。このネットワークでは、二つのユーザがそれぞれ二つづつ VM を使っていますが、それぞれが同じ IP アドレスを使っています。ホスト A がホスト B と通信を開始する際に、ホスト B の MAC アドレスを問い合わせます。当然のことながら、ホスト B の MAC アドレスはまだわかっていないので、ARP リクエストの宛先 MAC アドレスは、ブロードキャストアドレス (FF:FF:FF:FF:FF:FF) が用いられます。そのため、この ARP リクエストは、ユーザ B が使っているホストにも届いてしまいます。その結果、ホスト B と同じ IP アドレスを持つホスト D にも届いてしまします。その結果、ARP リプライは、ホスト B とホスト D の両方から返されることになるため、正しく通信ができません。

//image[arp_entangle][ARP リプライが混乱する]

=== ARPの制御のイメージ

//image[arp_block][同じユーザのホストにだけ ARP を届ける]

//image[translate][ARP リクエストは、宛先を書き換えて、別のホストへと届けられる]

Wakame-VDCは、エッジスイッチにOpen vSwitch (OVS)を利用しています。一つの例として、ARPがどのように届くのかを順を追って説明していきます。

（紙芝居）

このように、２台の仮想マシンの間にあるOVS間では、ARPのブロードキャストは物理ネットワークでは使われません。ARPは、OVSに指定されたルールによって物理ネットワークをユニキャストで通過するように変換されます。受け取った側のOVSは、元のARPに戻します。それから、ARPを目的の仮想マシンのNICだけに、あたかもブロードキャストされたかのように届けるのです。

これで、送った側の仮想マシンにも、受け取った側の仮想マシンにも、今までのARPと同じ挙動をしたように見せることができます。他の仮想マシンには一切分かりません。もし、他の仮想マシンが、ARPを偽装したとしても、SRCとDSTが一致しないものはDROPしてしまいます。

=== パケットの変換方法

仮想マシンが生成する仮想ネットワーク用のパケットを、物理ネットワーク向けに変換する方法は、色々あります。

最も有名な方法は、GREに代表される、トンネリング技術を使う方法です。実際は、仮想ネットワーク用のパケットそのものは書き換えず、物理ネットワークを通れる別のパケットを用意し、そのペイロードに包む方法です。このような手法をEncapsulationと言います。

Wakame-VDCでは、GREトンネルを使う方法も実験的に組み込まれていますが、より手軽で、高速な手法として、ARPパケットの制御を行う方法を紹介します。この仕組みは、ARPパケットのブロードキャストを制御し、目的となる仮想マシンのNICにだけ、的確にARPを届けることができれば、他の仮想マシンから見えなくなると言う原則に基づいています。

== Wakame-VDCの構成概要

ここまでは、仮想ネットワークについて注目した説明でした。Wakame-VDCは、全体の論理構成を図hogehogeのように、まとめています。

(全体の図)

Web UIから指示を受けて、Data Center Manager (dcmgr)と呼ばれるデータセンター全体のリソース管理をする中枢部分に司令が飛ぶと、実際に仮想マシンの準備や、仮想ネットワークの設定などが行われます。

必要な作業指示は、メッセージキュープロトコルであるAMQPのネットワークに流され、物理リソースの管理をしているエージェントに届き、処理されます。エージェントは、物理サーバに分散してインストールされており、各々が指示を受け取るようになっています。

特に、仮想マシンを収容するサーバで動作するエージェントは、Hyper Visor Agent (hva)と呼ばれており、これが仮想マシンを起動し、仮想ネットワークをセットアップします。

Wakame-VDCは、各物理サーバに常駐するhvaが、Tremaフレームワークを通じて、hvaと対になるようにローカルに配置されているOVSと連携します。つまり、Wakame-VDCは、分散するOpenFlowコントローラへ指示を出して、各コントローラが担当するOVSの設定をOpenFlowプロトコルで書き換えていると表現することができます。この辺りも、Tremaを活用した例としても特徴的な部分ではないでしょうか。

== なぜTremaを使い始めたのか

もともとWakame-VDCは、エッジスイッチとしてLinux BridgeのNetfilterを利用していました。物理サーバのホストOSに備わっているパケット制御の機構を用いて、ネットワークの操作をしていたのです。すでに、2010年11月には、いくつかの制約はあるものの、物理ネットワークの上に、仮想ネットワークを自由に組めるようになっていました。アーキテクチャも当時と変わっていません。

しかし、ホストOSに組み込まれた機能を使っているため、通信する度に、物理サーバのCPUを使ってしまいます。物理サーバには、仮想マシンも収容されていますので、仮想マシンのためにもCPUが使いたいのですが、今後はネットワークにもCPUを食われるようになりそうな懸念があったのです。未来のCPUの機能改善や、性能向上に期待しつつも、我々に出来ることは何かを模索していました。ネットワークの処理をオフロードする必要があると考えたのです。

その時に出会ったのが、OpenFlowでした。スイッチを制御する標準的なプロトコルとして、採用されれば、今ホストOS上で行なっている処理を、外部に出しても、OpenFlowを使って指示をすることはできます。オフロードを目論む私達にはぴったりの技術でした。あとは、OpenFlowコントローラとして、作りこみが必要だったので、フレームワークを探していたところ、Tremaが期待の星のように目の前に現れたのです。Wakame-VDCは、全てRubyで記述されていたのも幸いでした。早速Tremaのソースコードを読み、まずはフレームワークとしてのデザインが気に入りました。それからどう使っていくか、方針を決め、取り掛かったのが、2011年の秋頃でした。その後、Tremaの機能改善のため、パッチを送るなどして、何とかWakame-VDCへの組み込みが終わり、今は応用範囲を、より完全な仮想ネットワークへと進展させています。

== まとめ

Wakame-VDCがTremaフレームワークを使って得られた他の効果もあります。それは、ソースコードのパッチを送って以来、Trema開発者の方々と仲良くなれた事です。たまに一緒に飲みに行きますが、集まればOpenFlowの未来や、ギークな激論で盛り上がるかと思いきや、誰も一切そんな話なんてしません。ただの楽しい飲んだくれでございます。

飲み会はともかく、ソースコードのパッチを送る事はとても大切です。取り分け、オープンソースライセンスのコードは、自分で改善を施す事ができて初めて本来の意味を持ちます。ダウンロードして来て、使えるか、使えないかを判断するだけではいけません。もしあなたが、本当に技術者であり、開発者なのであれば、使えないと放棄する前に、使えるようにするためのコードを生み出さねばなりません。そうする事で世の中は一歩ずつ前に進んでいきます。さあ、パッチを書きましょう！あなたの力が世界を変えるのです。
