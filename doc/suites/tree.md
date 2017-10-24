## Tree test suite

Acceps a single parameter *n*: the number of vertices.

Note that the returned tree may have less than *n* vertices.

##### Completely random trees
1. random1
2. random2
3. random3
##### Trees generated with Kruskal-like approach
4. kruskal1
5. kruskal2
6. kruskal3
##### Trees generated with Prim-like approach with different elongations (from -100 to 100)
7. random_w-100
8. random_w-50
9. random_w-10
10. random_w-5
11. random_w-0
12. random_w-5
13. random_w-10
14. random_w-50
15. random_w-100
##### Bamboos and like
16. bamboo: bamboo 0-1-2-...-(n-1).
17. shuffled_bamboo: bamboo with random order of vertices.
18. 3branches: three branches of similar size connected to a single vertex.
19. sqrt_branches: sqrt(n) branches of the size sqrt(n) connected to a single vertex.
20. branches_123: branches of size 1, 2, ..., sqrt(n), connected to a single vertex.
##### Regular trees
21. binary
22. 3ary
23. 4ary
24. 50ary
25. 500ary
##### Stars and like
26. star: star with 0 at the center.
27. shuffled_star: star with random vertex at the center.
28. caterpillar_len90: bamboo of length n*0.9 with some vertices linked to random vertices on the path.
29. caterpillar_len50: same of length n*0.5.
30. caterpillar_len10: same of length n*0.1.
31. broom_n/2: bamboo of length n/2 connected to a star of size n/2.
