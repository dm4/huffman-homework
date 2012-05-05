VLC Decoding
============

<div style="float: right;">
R00922006 資工碩一 李孟翰
</div>

Usage
-----

-   Compile

        make

-   Execute

        ./r00922006_vlc_dec.exe huff_table_name compressed.txt decoded_symbol.txt

File Format
-----------

-   Huffman table

    每個 symbol-codeword pair 為兩行，第一行為 symbol ，第二行為 codeword ， symbol 和 codeword 都以 character 的方式儲存。

-   Compressed text file

    以 character 儲存的 bit stream ，如同助教給的 `compressed.txt` ，為了測式方便，在讀入 bit stream 時，如果遇到換行會自動忽略，因此不論 input file 是一個 symbol codeword 一行或是所有 bit stream 在同一行都能順利 decode。

-   Decoded text file

    一個 symbol 一行，以 character 型式儲存於 file 中，如同助教給的 `decoded_symbol.txt` 。

Modification
------------

在 [Memory Efficient and High-Speed Search Huffman Coding][1] 裡提到， LUT (look up table) 裡的 entry 個數是變動的，為了方便實作，我把每個 LUT 裡都放了 16 個 entry ，這樣雖然會在 LUT 多花一些空間，但對於 SGH-Tree (single-side growing Huffman Tree) 來說，entry 個數 < 16 個的 LUT 出現機率並不高 (<0.5) ，故這樣子的實作我認為是可以接受的。

Improvement
-----------

在 [Memory Efficient and High-Speed Search Huffman Coding][1] 裡，有個 Super-Table 的 data structure ，在我的實作是直接在 LUT 裡的，這點也是我覺得 paper 裡很奇怪的一點，為什麼不能把需要的資料直接寫在 LUT 裡，而需要去另外查表？

另外 paper 用了一個 sparse 的 symbol table ，在我的實作裡，用 std::vector 使用了一個 condense symbol table ，在 LUT 裡則是存了 symbol table element 的 index 。

照理來說要做壓縮，應該是要壓成 0101... 的 bit stream ，不過因為作業的 sample input (compressed.txt) 是將 bit stream 存成 character ，因為在我的實作裡也是把 codeword 及 symbol 都存成 character 的型式。

Other Implementation
--------------------

在實作 [Memory Efficient and High-Speed Search Huffman Coding][1] 時，為了測式方便，我也順便把 paper 中的範例 Huffman Table 建好，存為 `K7` ，也寫一個範例的 `compressed_K7.txt` ，裡面是所有 `K7` symbol 壓縮後的 bit stream 。

Bonus
=====

Bonus 的部份，我另外實作了 VLC encoder 以及相對應的 decoder ，說明如下：

Usage
-----

-   Compile

        make

-   Execute

        ./encoder input.txt  output.bin
        ./decoder output.bin output.txt

Implementation
--------------

在這次的實作裡，因為需要計算實際的壓縮率，所以我不再以 text file 當作 output ，而是真的以 bit stream 寫入檔案裡。

-   Encoder

    在 encode 的時候，我會以 byte 為單位，一個一個讀入，並計算出現次數（檔案結束以 0 表示），之後會依照 [Memory Efficient and High-Speed Search Huffman Coding][1] 裡提供的 algorithm ，先算出 TOCL (Table of Codeword Length) ，之後再 assign codeword ，最後寫檔的部份，我是先寫入 symbol 個數，再把 symbol table (symbol-codeword) 整個 struct 寫進去，接著把 encode 過後的 bit stream 寫入。

-   Decoder

    Decode 的時候，先讀出 symbol 的個數，接著把一個個的 symbol 讀出來，建好 symbol-codeword table ，接著就把 bit stream 讀出來，利用 [Memory Efficient and High-Speed Search Huffman Coding][1] 的方法去做 decode ，在這裡我也是以 4-bit 為一個單位來查表。

Analysis
--------

對於助教給的 `world95.txt` ，壓縮率是 1920605/2988578 ≑ 64% ，不過由於我是直接把 symbol table 寫進去，所以在 input text file 很小的話，壓起來的檔案反而會因為 symbol table 而變大，這裡我有想到幾個解決辦法：

-   先判斷一下壓縮後的檔案大小及 symbol table 大小，如果壓縮之後反而變大就乾脆不要壓縮，只是這樣就要在檔案一開始多用一個 bit 來表示這個檔案有沒有壓過。
-   不要把 symbol-codeword table 整個寫進去，而是寫 symbol 以及 symbol count ，而在 decode 的時候，再以同樣的 algorithm 建出 codeword ，而 symbol count table 所佔的空間是比 symbol-codeword table 小的。

不過以上兩點我在這次的作業裡並沒有實作。

Discussion
----------

在寫 encoder, decoder 的時候，我並沒有處理到 binary input file 的問題，因為我直接把 '\0' 當作 end of file 的 symbol ，但是在 binary file 裡有可能出現某個 byte 的值就是 '\0' ，這樣程式就不會正常執行了，我想到的解法是，以 byte 為單位的話，檔案裡可能的結果是 0~255 ，那麼我就多一個 256 的 symbol 當作 end of file ，不過這樣一來，本來存 symbol 只需要一個 byte ，加了 end of file 之後就需要 2-byte 了。

[1]:http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=469442&tag=1
