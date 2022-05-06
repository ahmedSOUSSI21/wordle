#ifndef MOTS_5_LETTRES_H
#define MOTS_5_LETTRES_H

/* Met dans ch un mot aléatoire de 5 lettres majuscules de la langue française,
 * suivi d'un caractère nul.
 * Précondition : ch doit pouvoir contenir 6 caractères. */
void mot_alea5(char *ch);

/* Retourne 1 si ch pointe vers une chaine de 5 caractères terminée par un 0 qui
 * apparait dans la liste de mots et 0 sinon */
int est_dans_liste_mots(const char *ch);

#endif /* ifndef MOTS_5_LETTRES_H */
