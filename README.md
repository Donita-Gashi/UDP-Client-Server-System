# UDP-Client-Server-System

## 📌 Përshkrimi

Ky projekt implementon një sistem komunikimi **Client-Server duke përdorur UDP** në gjuhën C, i kombinuar me një **server HTTP për monitorim në kohë reale**.

Serveri UDP pranon dhe përpunon mesazhe nga klientë të shumtë, ndërsa serveri HTTP ofron një mënyrë të thjeshtë për të parë statistikat dhe aktivitetin e sistemit përmes shfletuesit.

---

## 🚀 Funksionalitetet

* Komunikim UDP ndërmjet klientit dhe serverit
* Mbështetje për shumë klientë njëkohësisht
* Ruajtja e të dhënave në **storage/**
* Menaxhimi i fajllave (file_manager.c)
* Kontrolli i lejeve të qasjes (permissions.c)
* Server HTTP për monitorim
* Shfaqja e statistikave në kohë reale

---

## 🛠️ Teknologjitë

* C (Programming Language)
* Socket Programming (UDP & TCP)
* POSIX Threads (pthread)
* HTTP (Basic Implementation)

---

## 📂 Struktura e projektit

```bash id="t8q9n2"
project/
│── client.c            # Klienti UDP
│── server.c            # Serveri kryesor
│── http_server.c       # Serveri HTTP për monitorim
│── file_manager.c      # Menaxhimi i fajllave
│── permissions.c       # Kontrolli i lejeve
│── common.h            # Struktura dhe variabla të përbashkëta
│── storage/            # Ruajtja e të dhënave
│── README.md
```

---

## ⚙️ Instalimi dhe Ekzekutimi

### 1. Kompilimi

```bash id="m1x0zb"
gcc server.c http_server.c file_manager.c permissions.c -o server -lpthread
gcc client.c -o client
```

### 2. Startimi i serverit

```bash id="q7k3we"
./server
```

### 3. Startimi i klientit

```bash id="v2l8rx"
./client
```

---

## 🌐 Monitorimi

Për të parë statistikat në kohë reale, hapni në browser:

```id="j3c8pd"
http://localhost:8080/stats
```

---

## 📊 Shembull

* Active clients
* Total messages
* Lista e klientëve
* Logs e mesazheve

---

## 👨‍💻 Autorët

* Zana Ugraj
* Zgjim Sejdiu
* Zymer Ahmetaj
* Donita Gashi

---

## 📄 Licenca

Ky projekt është realizuar për qëllime akademike.
