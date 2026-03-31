# Multithread Web Server
**Многопоточный** статический HTTP веб-сервер. Обрабатывает и парсит приходящие HTTP запросы, герерирует HTTP ответы. Используется пул потоков. Сервер конфигурируется через ini файл

## Основные особенности
1) **Собственный однопроходынй** HTTP парсер, основаный на FSM
2) Использование **пула потков** для оптимизации работы под большими нагрузками
3) Применение **оптимизаций** под разные платформы (например, системный вызов `sendfile`)
4) Кроссплатформенность*
5) Режим `keep-alive` (тестовый)
6) **Собственный** парсер ini файла конфигурации

\* Пока что частичная

## В разработке
1) Внедрение механизмов *epoll, kqueue* для улучшения производительности
2) Полноценная поддержка режима `keep-alive`
3) Поддержка Windows Sockets API
4) Полноценное логирование

## Технологии
- Чистый C++23
- Google Test
- fmt
- CMake

## Запуск
``` bash
git clone https://github.com/ArtenKorolev/MultithreadWebServer
cd MultithreadWebServer
mkdir build
cd build
cmake .. -G<generator>
```

## Бенчмарки
**Мой сервер**

<img width="594" height="181" alt="изображение" src="https://github.com/user-attachments/assets/1c0db70d-a8e3-43ef-b717-24ff3404e7ed" />

**nginx на этом же компьютере**

<img width="595" height="181" alt="изображение" src="https://github.com/user-attachments/assets/401c9d95-e592-46c8-8e21-0fc8635482cc" />

