Labo Unix 2023

Ce projet regroupe une série d'exercices pratiques réalisés dans le cadre d'un cours d'informatique, explorant les concepts avancés de la programmation sous Unix. Il met l'accent sur la gestion des processus, les threads, et les mécanismes de synchronisation tels que les mutex, les sémaphores, et la mémoire partagée.

📁 Structure du Projet

Le projet est organisé en plusieurs exercices abordant des thèmes spécifiques de la programmation Unix :

    Exercice 1
    Introduction à la gestion des processus et à l'utilisation des scripts de compilation (makefile).

    Exercice 2
    Gestion des fichiers structurés pour manipuler les données utilisateurs (lecture, écriture, modification).

    Exercice 3
    Gestion des données partagées avec un accent sur la synchronisation et la prévention des conflits.

    Exercice 4
    Mise en œuvre d'un système multitâche avec journalisation, intégrant :
        Threads pour exécuter plusieurs tâches en parallèle.
        Mutex et sémaphores pour synchroniser l'accès aux ressources partagées.

    Exercice 5
    Exploration avancée de la mémoire partagée pour coordonner plusieurs processus travaillant sur des ressources communes.

🛠️ Fonctionnalités Techniques

    Gestion des Processus :
        Création et contrôle des processus via les primitives Unix telles que fork et exec.

    Threads et Synchronisation :
        Mutex pour protéger les sections critiques et prévenir les accès concurrents.
        Sémaphores pour coordonner les accès à des ressources limitées.
        Utilisation de conditions pour gérer les files d'attente ou les signaux entre threads.

    Mémoire Partagée :
        Création et gestion de segments de mémoire partagée pour permettre aux processus d'échanger des données efficacement.

    Journalisation et Traces :
        Enregistrement des événements dans des fichiers log pour surveiller l'exécution et diagnostiquer les erreurs.

🚀 Technologies Utilisées

    Langages : C++
    API Unix/Linux :
        Threads (pthread) pour le multitâche.
        Mutex et sémaphores pour la synchronisation.
        Mémoire partagée pour l'IPC.
    Automatisation :
        Fichiers makefile pour faciliter la compilation.
        Scripts Bash pour des tâches spécifiques.

📦 Installation et Exécution

    Cloner le dépôt :

git clone <url_du_projet>
cd unix-main

Compiler un exercice : Accédez au dossier de l'exercice souhaité et lancez le script de compilation :

    ./Compile.sh

    Exécuter le programme : Les instructions spécifiques pour chaque exercice sont disponibles dans leur README.md.

📚 Organisation des Fichiers

    main.cpp : Point d'entrée principal pour chaque exercice.
    makefile : Automatisation des étapes de compilation.
    Fichiers de Synchronisation : Code implémentant les mutex, sémaphores et mémoire partagée.
    Fichiers Logs : Capturent les événements système et les erreurs.
    Ressources Partagées :
        Segments de mémoire partagée pour l'échange de données.
        Sémaphores pour la gestion de l'accès aux ressources.

🖊️ Auteur

Ce projet a été développé pour explorer les mécanismes avancés de la programmation Unix, en mettant l'accent sur la gestion des processus, des threads et des ressources partagées.
