# Navier-Stokes Solver — Schéma global et résumé

## 1. Variables et problème étudié

Le solveur travaille sur un maillage triangulaire 2D et utilise une discrétisation par éléments finis P1.

Les inconnues principales sont :

- `omega` : la vorticité $\omega$ ;
- `psi` : la fonction de courant $\psi$ ;
- `M` : la matrice de masse ;
- `S` : la matrice de rigidité ;
- `Momega = M \omega` : la vorticité intégrée contre les fonctions de base.

Le système repose sur la formulation vorticité–fonction de courant :

$$
\omega = -\Delta \psi
$$

et sur l'équation de transport-diffusion de la vorticité :

$$
\frac{\partial \omega}{\partial t}
+ \mathbf{u}\cdot\nabla\omega
=
\nu\Delta\omega.
$$

La vitesse est reconstruite à partir de la fonction de courant :

$$
\mathbf{u}
=
\begin{pmatrix}
u_x\\
u_y
\end{pmatrix}
=
\begin{pmatrix}
\partial_y\psi\\
-\partial_x\psi
\end{pmatrix}.
$$

---

# 2. Construction du solveur

## Étape 1 — Construction des matrices éléments finis

Au début, le solveur construit :

$$
M
$$

la matrice de masse, et :

$$
S
$$

la matrice de rigidité.

Dans le code :

```cpp
build_P1_CSRPattern(m, P);
build_P1_mass_matrix(m, P, M);
build_P1_stiffness_matrix(m, P, S);
```

Le solveur utilise ici le format creux CSR.

La quantité :

$$
\mathrm{vol} = \sum_{i,j} M_{ij}
$$

correspond à la mesure totale représentée par le maillage : aire en 2D.

---

# 3. Schéma global d'un pas de temps

Le déroulement général de `time_step(dt, nu)` est :

```text
omega(t), psi(t)
      |
      v
+------------------------------+
| 1. Calcul de la fonction     |
|    de courant psi            |
|                              |
|    S psi = M omega           |
+------------------------------+
      |
      v
+------------------------------+
| 2. Calcul du terme de         |
|    transport T(omega, psi)    |
+------------------------------+
      |
      v
+------------------------------+
| 3. Résolution implicite de    |
|    diffusion                  |
|                              |
| (M + nu dt S) omega(t+dt)     |
| = M omega(t) + dt T           |
+------------------------------+
      |
      v
+------------------------------+
| 4. Correction de moyenne      |
|    nulle de omega             |
+------------------------------+
      |
      v
omega(t + dt)
      |
      v
t <- t + dt
```

---

# 4. Étape 1 — Calcul de la fonction de courant

La relation continue est :

$$
\omega = -\Delta\psi.
$$

Après discrétisation par éléments finis, le problème devient :

$$
S\psi = M\omega.
$$

Le membre de droite est d'abord calculé :

$$
b = M\omega.
$$

Dans le code :

```cpp
M.mvp(Om, MOm);
```

Puis le solveur utilise le gradient conjugué pour résoudre :

$$
S\psi = M\omega.
$$

Le résidu initial est :

$$
r_0 = M\omega - S\psi_0.
$$

Puis :

$$
p_0 = r_0.
$$

À chaque itération du gradient conjugué :

$$
A_p = S p_k,
$$

$$
\alpha_k =
\frac{r_k^T r_k}
     {p_k^T S p_k},
$$

$$
\psi_{k+1}
=
\psi_k + \alpha_k p_k,
$$

$$
r_{k+1}
=
r_k - \alpha_k S p_k,
$$

$$
\beta_k
=
\frac{r_{k+1}^T r_{k+1}}
     {r_k^T r_k},
$$

$$
p_{k+1}
=
r_{k+1}+\beta_k p_k.
$$

Le critère de convergence utilisé est :

$$
\frac{\|r_k\|}{\|b\|}
\leq \texttt{tol}.
$$

---

# 5. Étape 2 — Calcul du transport

Une fois $\psi$ calculée, le solveur construit le terme non linéaire de transport :

$$
T(\omega,\psi).
$$

Pour chaque triangle $(a,b,c)$, le code calcule :

$$
s = \omega_a+\omega_b+\omega_c.
$$

Puis ajoute :

$$
T_a
\mathrel{+}=
\frac{1}{6}s(\psi_b-\psi_c),
$$

$$
T_b
\mathrel{+}=
\frac{1}{6}s(\psi_c-\psi_a),
$$

$$
T_c
\mathrel{+}=
\frac{1}{6}s(\psi_a-\psi_b).
$$

Ce terme représente la discrétisation du transport de la vorticité par le champ de vitesse reconstruit à partir de $\psi$.

Propriété importante :

$$
T_a+T_b+T_c=0
$$

pour chaque triangle. Le transport ne crée donc pas de moyenne globale de vorticité.

---

# 6. Étape 3 — Avancement temporel de la vorticité

L'équation continue est :

$$
\frac{\partial\omega}{\partial t}
+
\text{transport}
=
\nu\Delta\omega.
$$

Le code utilise une discrétisation temporelle de type Euler implicite pour la diffusion.

Le système discret résolu est :

$$
\left(M+\nu\Delta t S\right)
\omega^{n+1}
=
M\omega^n
+
\Delta t\,T(\omega^n,\psi^n).
$$

Le membre de droite est :

$$
b
=
M\omega^n
+
\Delta t\,T^n.
$$

Dans le code :

```cpp
compute_transport(P);
blas_axpby(1, MOm, dt, P, N);
```

La matrice du système est :

$$
A
=
M+\nu\Delta t S.
$$

Le gradient conjugué résout alors :

$$
A\omega^{n+1}=b.
$$

À chaque itération :

$$
A_p
=
\left(M+\nu\Delta t S\right)p_k,
$$

puis les mêmes formules du gradient conjugué sont appliquées.

---

# 7. Étape 4 — Condition de moyenne nulle

La matrice de rigidité associée au Laplacien possède une fonction constante dans son noyau.

En effet :

$$
S\mathbf{1}=0.
$$

La solution n'est donc pas définie de manière unique à une constante additive près.

Le code impose une moyenne discrète nulle.

Pour un vecteur $V$, il calcule :

$$
s
=
\sum_i(MV)_i.
$$

La moyenne est :

$$
\bar V
=
\frac{s}{\mathrm{vol}}.
$$

Puis :

$$
V
\leftarrow
V-\bar V.
$$

Ainsi :

$$
\sum_i(MV)_i=0.
$$

Cette opération est utilisée pour maintenir une vorticité compatible avec le problème.

---

# 8. Résumé complet du calcul

Pour passer de $t^n$ à $t^{n+1}$ :

### 1. Vorticité connue

$$
\omega^n
$$

### 2. Calcul de la fonction de courant

$$
S\psi^n=M\omega^n
$$

### 3. Reconstruction du transport

$$
T^n=T(\omega^n,\psi^n)
$$

### 4. Construction du membre de droite

$$
b=M\omega^n+\Delta t T^n
$$

### 5. Résolution du système d'évolution

$$
(M+\nu\Delta tS)\omega^{n+1}=b
$$

### 6. Correction de moyenne

$$
\omega^{n+1}
\leftarrow
\omega^{n+1}
-
\frac{\sum_i(M\omega^{n+1})_i}{\mathrm{vol}}
$$

### 7. Mise à jour du temps

$$
t^{n+1}=t^n+\Delta t.
$$

---

# 9. Correspondance avec les fonctions du code

| Fonction | Rôle |
|---|---|
| `NavierStokesSolver()` | Initialise le maillage, les vecteurs et les matrices FEM |
| `set_zero_mean()` | Retire la moyenne discrète d'un vecteur |
| `compute_transport()` | Assemble le terme de transport triangle par triangle |
| `compute_stream_function()` | Résout $S\psi=M\omega$ par gradient conjugué |
| `time_step(dt, nu)` | Effectue un pas de temps complet |
| `M.mvp()` | Calcule un produit matrice-vecteur avec la matrice de masse |
| `S.mvp()` | Calcule un produit matrice-vecteur avec la matrice de rigidité |

---

# 10. Idée générale

Le solveur ne résout pas directement la vitesse et la pression.

Il utilise la formulation :

$$
\boxed{\text{vorticité } \omega}
\longrightarrow
\boxed{\text{fonction de courant } \psi}
\longrightarrow
\boxed{\text{transport}}
\longrightarrow
\boxed{\text{nouvelle vorticité } \omega}.
$$

Le cycle complet est donc :

$$
\boxed{
\omega^n
\xrightarrow{S\psi=M\omega}
\psi^n
\xrightarrow{\text{transport}}
T^n
\xrightarrow{(M+\nu\Delta tS)\omega^{n+1}=M\omega^n+\Delta tT^n}
\omega^{n+1}
}
$$

Ce cycle est répété à chaque pas de temps.
